internal b32
UI_IsNilBox(ui_box *Box)
{
    b32 Result = (Box == 0) || (Box == UI_NilBox);
    return Result;
}

internal void
UI_PushBox(void)
{
    // NOTE(luca): You may not double push cause it makes no sense
    Assert(!AppendToParent);
    
    AppendToParent = true;
}

internal ui_key
UI_KeyNull(void)
{
    ui_key Result = {};
    return Result;
}

internal b32
UI_KeyMatch(ui_key A, ui_key B)
{
    b32 Result = (A.U64[0] == B.U64[0]);
    return Result;
}

internal ui_box *
UI_AddBox(str8 DisplayString, u32 Flags, ui_size Size[Axis2_Count])
{
#if 0    
    // TODO(luca): Seed with parent key and avoid null boxes and keys.
    //1. Take the current pattern, if they don't have a null key then we can use it
    //2. Otherwise go to parent's parent
    //3. Step 1
#else
    u64 AncestorKey = UI_Current->Parent->Key.U64[0];
#endif
    ui_key Key = {U64HashFromSeedStr8(AncestorKey, DisplayString)};
    ui_box *Box = 0;
    b32 FirstTime = true;
    
    // Get the box based on key
    {    
        u64 Slot = (Key.U64[0] % UI_BoxTableSize);
        ui_box *HashBox = UI_BoxTable + Slot;
        ui_box *HashLast = 0;
        
        // There are three scenario's
        //1. We find the slot empty
        //2. We find the slot occupied
        //2.1 The key matches the slot or one from the linked list
        //2.2 The key did not match
        
        if(UI_KeyMatch(UI_KeyNull(), HashBox->Key))
        {
            // 1.
            Box = HashBox;
            
            Box->First = Box->Last = Box->Next = Box->Prev = Box->Parent = UI_NilBox;
            Box->HashNext = Box->HashPrev = UI_NilBox;
        }
        else
        {     
            while(!UI_IsNilBox(HashBox))
            {        
                if(UI_KeyMatch(HashBox->Key, Key))
                {
                    break;
                }
                
                HashLast = HashBox;
                HashBox = HashBox->Next;
            }
            
            if(!UI_IsNilBox(HashBox))
            {
                //2.1
                Box = HashBox;
                FirstTime = false;
            }
            else
            {
                //2.2
                HashLast->HashNext = PushStruct(UI_BoxArena, ui_box);
                Box = HashLast->HashNext;
                Box->HashPrev = HashLast;
            }
        }
        
    }
    
    Box->Key = Key;
    Box->DisplayString = DisplayString;
    Box->Flags = Flags;
    Box->SemanticSize[Axis2_X] = Size[Axis2_X];
    Box->SemanticSize[Axis2_Y] = Size[Axis2_Y];
    
    // TODO(luca): Override these default settings.
    Box->BackgroundColor = Color_ButtonBackground;
    Box->BorderColor = Color_ButtonBorder;
    Box->TextColor = Color_ButtonText;
    Box->CornerRadii = V4(5.f, 5.f, 5.f, 5.f);
    Box->BorderThickness = 2.f;
    Box->Softness = .5f;
    
    if(!FirstTime)
    {
        v2 MouseP = V2S32(UI_Input->MouseX, UI_Input->MouseY);
        
        b32 Hovered = IsInsideRectV2(MouseP, RectFromSize(Box->FixedPosition, Box->FixedSize));
        b32 Clicked = (Hovered && (WasPressed(UI_Input->MouseButtons[PlatformMouseButton_Left]))); 
        b32 Pressed = (Hovered && UI_Input->MouseButtons[PlatformMouseButton_Left].EndedDown);
        
        Box->Clicked = Clicked;
        Box->Hovered = Hovered;
        Box->Pressed = Pressed;
    }
    
    // Add box to the tree
    {    
        if(AppendToParent)
        {
            UI_Current->First = Box;
            Box->Parent = UI_Current;
        }
        else
        {    
            Box->Parent = UI_Current->Parent;
            
            if(!UI_IsNilBox(Box->Parent))
            {
                Box->Parent->Last = Box;
            }
            
            Box->Prev = UI_Current;
            UI_Current->Next = Box;
        }
        
        UI_Current = Box;
        
        AppendToParent = false;
    }
    
    return Box;
}

internal void
UI_PopBox()
{
    UI_Current = UI_Current->Parent;
}

internal f32
UI_MeasureTextWidth(str8 String)
{
    f32 Result = 0.f;
    
    for EachIndex(Idx, String.Size)
    {
        u8 Char = String.Data[Idx];
        f32 CharWidth = (UI_Atlas->PackedChars[Char].xadvance);
        
        Result += CharWidth;
    }
    
    return Result;
}

internal void
UI_DrawBoxes(ui_box *Box)
{
    f32 BorderWidth = Box->BorderThickness;
    f32 Softness = Box->Softness;
    v4 DebugBorderColor = V4(0.f, 0.f, 1.f, 1.f);;
    
    // NOTE(luca): We might not have to special case the root element since it has references to the NilBox
    if(Box != UI_Root)
    {
        if(Box->Parent->First == Box)
        {
            Box->FixedPosition.X = Box->Parent->FixedPosition.X;
            Box->FixedPosition.Y = Box->Parent->FixedPosition.Y;
        }
        else
        {            
            Box->FixedPosition.Y = Box->Parent->FixedPosition.Y;
            Box->FixedPosition.X = (Box->Prev->FixedPosition.X + 
                                    Box->Prev->FixedSize.X);
        }
        
        for EachIndex(Idx, Axis2_Count)
        {
            ui_size SemanticSize = Box->SemanticSize[Idx]; 
            f32 *FixedSize = &Box->FixedSize.e[Idx];
            
            if(0) {}
            else if(SemanticSize.Kind == UI_SizeKind_Pixels)
            {
                *FixedSize = SemanticSize.Value;
            }
            else if(SemanticSize.Kind == UI_SizeKind_TextContent)
            {
                if(Idx == Axis2_X)
                {
                    *FixedSize = (UI_MeasureTextWidth(Box->DisplayString) + 
                                  2.f*BorderWidth);
                }
                else
                {
                    *FixedSize = (UI_Atlas->HeightPx + 
                                  2.f*BorderWidth);
                }
            }
            else if(SemanticSize.Kind == UI_SizeKind_PercentOfParent)
            {
                *FixedSize = Box->Parent->FixedSize.e[Idx] * SemanticSize.Value;
            }
        }
    }
    
    rect Dest = RectFromSize(Box->FixedPosition, Box->FixedSize);
    
    if(Box->Flags & UI_BoxFlag_DrawBackground)
    {    
        rect_instance *Inst = DrawRect(Dest, Box->BackgroundColor, 0.f, 0.f, Softness);
        V4Math Inst->CornerRadii.E = Box->CornerRadii.E;
        
        if(Box->Flags & UI_BoxFlag_AnimateColorOnHover)
        {
            if(Box->Hovered)
            {
                Inst->Color0.W = .2f;
                Inst->Color1.W = .2f;
                
                if(Box->Pressed)
                {
                    Inst->Color0.W = 1.f;
                    Inst->Color1.W = 1.f;
                    Inst->Color2.W = .2f;
                    Inst->Color3.W = .2f;
                }
                
            }
        }
    }
    
    if(Box->Flags & UI_BoxFlag_DrawDisplayString)
    {
        v2 TextPos = Box->FixedPosition;
        TextPos.Y += BorderWidth;
        TextPos.X += BorderWidth;
        
        if(Box->Flags & UI_BoxFlag_CenterTextHorizontally)
        {
            f32 TextWidth = UI_MeasureTextWidth(Box->DisplayString);
            TextPos.X += .5f*((Box->FixedSize.X - 2.f*BorderWidth) - TextWidth);
        }
        
        if(Box->Flags & UI_BoxFlag_CenterTextVertically)
        {
            f32 TextHeight = UI_Atlas->HeightPx;
            TextPos.Y += .5f*((Box->FixedSize.Y - 2.f*BorderWidth) - TextHeight);
        }
        
        for EachIndex(Idx, Box->DisplayString.Size)
        {
            rune Char = Box->DisplayString.Data[Idx];
            
            f32 CharWidth = (UI_Atlas->PackedChars[Char].xadvance);
            
            DrawRectChar(UI_Atlas, TextPos, Char, Color_ButtonText);
            
            TextPos.X += CharWidth;
        }
    }
    
    if(Box->Flags & UI_BoxFlag_DrawBorders)
    {    
        rect_instance *Inst = DrawRect(Dest, Box->BorderColor, 0.f, BorderWidth, Softness);
        V4Math Inst->CornerRadii.E = Box->CornerRadii.E;
    }
    
    if(UI_GlobalDebugMode || Box->Flags & UI_BoxFlag_DrawDebugBorder)
    {    
        DrawRect(Dest, DebugBorderColor, 0.f, 1.f, 0.f);
    }
    
    if(!UI_IsNilBox(Box->Next))
    {
        UI_DrawBoxes(Box->Next);
    }
    
    if(!UI_IsNilBox(Box->First))
    {
        UI_DrawBoxes(Box->First);
    }
    
}
