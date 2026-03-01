                       internal v4
ColorForTokenType(token_type Type)
{
    switch(Type)
    {
        case TokenIdentifier:              return Color_Green;
        case TokenIdentifierAssignmentValue:
        case TokenValue:                   return Color_Yellow;
        case TokenNumber:                  return Color_Orange;
        case TokenFunc:                    return Color_Blue;
        default:                           return Color_Red;
    }
}

internal str8
LabelForTokenType(token_type Type)
{
    switch(Type)
    {
        case TokenIdentifier:               return S8("ident");
        case TokenIdentifierAssignmentValue:return S8("asgn");
        case TokenValue:                    return S8("val");
        case TokenNumber:                   return S8("num");
        case TokenString:                   return S8("str");
        case TokenFunc:                     return S8("func");
        case TokenReturn:                   return S8("ret");
        case TokenIf:                       return S8("if");
        case TokenElse:                     return S8("else");
        case TokenFor:                      return S8("for");
        case TokenWhile:                    return S8("while");
        case TokenBreak:                    return S8("break");
        case TokenContinue:                 return S8("cont");
        case TokenDoubleEqual:              return S8("==");
        case TokenGreaterEqual:             return S8(">=");
        case TokenLesserEqual:              return S8("<=");
        case TokenLeftShift:                return S8("<<");
        case TokenRightShift:               return S8(">>");
        case TokenParam:                    return S8("param");
        case TokenExpression:               return S8("expr");
        default:                            return S8("?");
    }
}

// Pass 1: recursively mirror the syntax tree into visual_node structs,
// assigning each node a column/row slot. Returns the subtree width in columns.
internal s32
BuildVisualTree(syntax_node  *SyntaxNode,
                visual_node  *VisualParent,
                visual_tree  *VTree,
                arena        *Arena,
                s32           Col,
                s32           Row,
                f32           NodeW,
                f32           NodeH,
                f32           PadX,
                f32           PadY)
{
    if(!SyntaxNode || SyntaxNode == &nil_syntax_node)
    {
        return Col;
    }

    visual_node *VNode = PushStruct(Arena, visual_node);
    VNode->Size        = V2(NodeW, NodeH);
    VNode->Color       = ColorForTokenType(SyntaxNode->Token->Type);
    VNode->Label       = LabelForTokenType(SyntaxNode->Token->Type);
    VNode->Parent      = VisualParent;
    VTree->Count++;

    if(VisualParent)
    {
        if(!VisualParent->First)
        {
            VisualParent->First = VNode;
            VisualParent->Last  = VNode;
        }
        else
        {
            VisualParent->Last->Next = VNode;
            VisualParent->Last       = VNode;
        }
    }
    else
    {
        VTree->Root = VNode;
    }

    s32 SubtreeCol = Col;

    if(SyntaxNode->First)
    {
        s32 ChildCol = Col;
        for(syntax_node *Child = SyntaxNode->First;
            Child && Child != &nil_syntax_node;
            Child = Child->NextNode)
        {
            ChildCol = BuildVisualTree(Child, VNode, VTree, Arena,
                                       ChildCol, Row + 1,
                                       NodeW, NodeH, PadX, PadY);
        }
        SubtreeCol = ChildCol;
    }
    else
    {
        SubtreeCol = Col + 1;
    }

    f32 LeftX  = (f32)Col             * (NodeW + PadX);
    f32 RightX = (f32)(SubtreeCol - 1) * (NodeW + PadX);
    VNode->Pos = V2((LeftX + RightX) * 0.5f, (f32)Row * (NodeH + PadY));

    return SubtreeCol;
}

// Pass 2: draw edges then nodes from an already-built visual_tree.
// Uses an explicit stack to avoid recursion limits on deep trees.
internal void
DrawVisualTree(visual_tree          *VTree,
               app_offscreen_buffer *Buffer,
               f32                   OffsetX,
               f32                   OffsetY,
               f32                   NodeW,
               f32                   NodeH)
{
    if(!VTree->Root) return;

    // Iterative depth-first traversal using First/Next links
    visual_node *Stack[1024];
    s32          Top   = 0;
    Stack[Top++]       = VTree->Root;

    while(Top > 0)
    {
        visual_node *VNode = Stack[--Top];

        v2 NodeCenter = V2(OffsetX + VNode->Pos.X + NodeW * 0.5f,
                           OffsetY + VNode->Pos.Y + NodeH * 0.5f);

        rect           Dest = RectFromSize(V2(OffsetX + VNode->Pos.X,
                                              OffsetY + VNode->Pos.Y),
                                           VNode->Size);
        rect_instance *Inst = DrawRect(Dest, VNode->Color, 0.f, 1.f, 1.f);
        Inst->Color0.W      = 1.f;
        Inst->Color1.W      = 1.f;
        Inst->Color2.W      = 1.f;
        Inst->Color3.W      = 1.f;

        // Push children right-to-left so leftmost is processed first
        for(visual_node *Child = VNode->Last; Child; )
        {
            Stack[Top++] = Child;
            // Walk backward -- find previous sibling by re-scanning from First
            if(Child == VNode->First) break;
            visual_node *Prev = VNode->First;
            while(Prev->Next != Child) Prev = Prev->Next;
            Child = Prev;
        }
    }
}

internal void
visualize_tree(concrete_syntax_tree *Tree,
              arena                *Arena,
              app_offscreen_buffer *Buffer,
              app_input            *Input)
{
    if(!Tree || !Tree->Root) return;

    f32 NodeW = 50.f;
    f32 NodeH = 50.f;
    f32 PadX  = 60.f;
    f32 PadY  = 60.f;

    f32 OffsetX = 100.f;
    f32 OffsetY = 100.f;

    visual_tree VTree = {0};

    BuildVisualTree(Tree->Root, NULL, &VTree, Arena,
                    0, 0, NodeW, NodeH, PadX, PadY);

    DrawVisualTree(&VTree, Buffer, OffsetX, OffsetY, NodeW, NodeH);
}
