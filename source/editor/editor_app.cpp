#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"
#include "editor_math.h"
#include "editor_platform.h"
#include "editor_font.h"
#include "editor_random.h"
#include "editor_libs.h"
#include "editor_gl.h"
#include "editor_ui.h"
#include "editor_app.h"

internal app_offscreen_buffer
LoadImage(arena *Arena, str8 ExeDirPath, str8 Path)
{
    app_offscreen_buffer Result = {};
    
    char *FilePath = PathFromExe(Arena, ExeDirPath, Path);
    str8 File = OS_ReadEntireFileIntoMemory(FilePath);
    if(File.Size)
    {
        s32 Width, Height, Components;
        s32 BytesPerPixel = 4;
        u8 *Image = stbi_load_from_memory(File.Data, (int)File.Size, &Width, &Height, &Components, BytesPerPixel);
        Assert(Components == BytesPerPixel);
        
        Result.Width = Width;
        Result.Height = Height;
        Result.BytesPerPixel = BytesPerPixel;
        Result.Pitch = Result.BytesPerPixel*Width;
        Result.Pixels = Image;
    }
    
    return Result;
}

internal inline u32 *
GetPixel(app_offscreen_buffer *Buffer, s32 X, s32 Y)
{
    u32 *Pixel = (u32 *)(Buffer->Pixels + Buffer->BytesPerPixel*(Y*Buffer->Width + X));
    return Pixel;
}

void
AppendChar(app_state *App, rune Codepoint)
{
    if (Codepoint == ' ')
    {
        // NOTE(nasr): do we check the space here?
    }
    App->Text[App->TextCount] = Codepoint;
    App->TextCount += 1;
    App->CursorPos += 1;
    Assert(App->TextCount < ArrayCount(App->Text));
}

void
DeleteChar(app_state *App)
{
    if(App->TextCount)
    {
        App->TextCount -= 1;
        App->CursorPos -= 1;
    }
}

internal inline b32
EqualsWithEpsilon(f32 A, f32 B, f32 Epsilon)
{
    b32 Result = (A < (B + 0.001f) && 
                  A > (B - 0.001f));
    return Result;
}

void
InitAtlas(arena *Arena, app_font *Font, font_atlas *Atlas, f32 HeightPx)
{
    Atlas->FirstCodepoint = ' ';
    // NOTE(luca): 2 bytes =>small alphabets.
    Atlas->CodepointsCount = ((0xFFFF - 1) - Atlas->FirstCodepoint);
    
    Atlas->Width = 1024;
    Atlas->Height = 1024;
    u32 Size = (Atlas->Width*Atlas->Height);
    
    Arena->Pos = 0;
    Atlas->Data = PushArray(Arena, u8, Size);
    Atlas->PackedChars = PushArray(Arena, stbtt_packedchar, Size);
    Atlas->AlignedQuads = PushArray(Arena, stbtt_aligned_quad, Size);
    
    Atlas->HeightPx = HeightPx;
    Atlas->FontScale = stbtt_ScaleForPixelHeight(&Font->Info, HeightPx);
    Atlas->Font = Font;
    
    stbtt_pack_context ctx;
    {
        stbtt_PackBegin(&ctx, 
                        Atlas->Data,
                        Atlas->Width, Atlas->Height,
                        0, 1, 0);
        
        stbtt_PackFontRange(&ctx, Font->Info.data, 0, HeightPx, 
                            Atlas->FirstCodepoint, Atlas->CodepointsCount, 
                            Atlas->PackedChars);
        stbtt_PackEnd(&ctx);
    }
    
    for EachIndex(Idx, Atlas->CodepointsCount)
    {
        float UnusedX, UnusedY;
        stbtt_GetPackedQuad(Atlas->PackedChars, Atlas->Width, Atlas->Height, 
                            (u32)Idx,
                            &UnusedX, &UnusedY,
                            &Atlas->AlignedQuads[Idx], 0);
    }
    
}

internal rect_instance *
DrawRect(rect Dest, v4 Color, f32 CornerRadius, f32 BorderThickness, f32 Softness)
{
    rect_instance *Result = GlobalRectsInstances + GlobalRectsCount;
    GlobalRectsCount += 1;
    
    MemoryZero(Result);
    
    Result->Dest = Dest;
    Result->Color0 = Color;
    Result->Color1 = Color;
    Result->Color2 = Color;
    Result->Color3 = Color;
    Result->CornerRadii.e[0] = CornerRadius;
    Result->CornerRadii.e[1] = CornerRadius;
    Result->CornerRadii.e[2] = CornerRadius;
    Result->CornerRadii.e[3] = CornerRadius;
    Result->Border = BorderThickness;
    Result->Softness = Softness;
    
    return Result;
}

internal rect_instance *
DrawRectChar(font_atlas *Atlas, v2 Pos, rune Codepoint, v4 Color)
{
    rect_instance *Result = GlobalRectsInstances + GlobalRectsCount;
    GlobalRectsCount += 1;
    
    MemoryZero(Result);
    
    // NOTE(luca): Evereything happens in pixel coordinates in here.
    
    rune CharIdx = Codepoint - Atlas->FirstCodepoint;
    stbtt_packedchar *PackedChar = &Atlas->PackedChars[CharIdx];
    f32 Width = (PackedChar->x1 - PackedChar->x0);
    f32 Height = (PackedChar->y1 - PackedChar->y0);
    {    
        v2 Min = Pos;
        Min.X += (PackedChar->xoff);
        f32 Baseline = GetBaseline(Atlas->Font, Atlas->FontScale);
        f32 Descent = (f32)Atlas->Font->Descent*Atlas->FontScale;
        Min.Y += (PackedChar->yoff) + Baseline + Descent;
        
        v2 Max = V2(Min.X + Width, Min.Y + Height);
        
        Result->Dest = Rect(Min.X, Min.Y, Max.X, Max.Y);
        
        stbtt_aligned_quad *Quad = &Atlas->AlignedQuads[CharIdx];
        Result->TexSrc = Rect(Quad->s0, Quad->t0, Quad->s1, Quad->t1);
    }
    
    Result->Color0 = Color;
    Result->Color1 = Color;
    Result->Color2 = Color;
    Result->Color3 = Color;
    Result->HasTexture = true;
    
#if 0    
    DrawRect(Result->Dest, V4(0.f, 0.f, 1.f, 1.f), 0.f, 1.f, 0.f);
#endif
    
    return Result;
}

#include "editor_ui.cpp"

C_LINKAGE
UPDATE_AND_RENDER(UpdateAndRender)
{
    b32 ShouldQuit = false;
    
#if EDITOR_INTERNAL
    GlobalDebuggerIsAttached = Memory->IsDebuggerAttached;
#endif
#if OS_WINDOWS
    GlobalPerfCountFrequency = Memory->PerfCountFrequency;
#endif
    
    ui_box *UI_Boxes;
    arena *PermanentArena, *FrameArena;
    app_state *App;
    {    
        PermanentArena = (arena *)Memory->Memory;
        PermanentArena->Size = Memory->MemorySize - sizeof(arena);
        PermanentArena->Base = (u8 *)Memory->Memory + sizeof(arena);
        PermanentArena->Pos = 0;
        AsanPoisonMemoryRegion(PermanentArena->Base, PermanentArena->Size);
        
        FrameArena = PushArena(PermanentArena, Memory->MemorySize/2);
        
        App = PushStruct(PermanentArena, app_state);
    }
    
    font_atlas *Atlas = &App->FontAtlas;
    
    // NOTE(luca): Will be rerun when reloaded.
    local_persist s32 GLADVersion = 0;
    if(!Memory->Initialized || Memory->Reloaded)
    {
        GLADVersion = gladLoaderLoadGL();
    }
    
    if(!Memory->Initialized)
    {
        char *FontPath = PathFromExe(FrameArena, Memory->ExeDirPath, S8("../data/font_regular.ttf"));
        InitFont(&App->Font, FontPath);
        App->FontAtlasArena = PushArena(PermanentArena, MB(100));
        App->PreviousHeightPx = DefaultHeightPx + 1.0f;
        App->HeightPx = DefaultHeightPx;
        
        App->TextCount = 0;
        App->CursorPos = 0;
        
        App->UIBoxTableSize = 4096;
        App->UIBoxTable = PushArray(PermanentArena, ui_box, App->UIBoxTableSize);
        App->UIArena = PushArena(PermanentArena, 256*sizeof(ui_box));
        
        App->TrackerForUI_NilBox = &_UI_NilBox;
        
        Memory->Initialized = true;
    }
    
    UI_NilBox = App->TrackerForUI_NilBox;
    
    // Text input
    for EachIndex(Idx, Input->Text.Count)
    {
        app_text_button Key = Input->Text.Buffer[Idx];
        
        if(!Key.IsSymbol)
        {
            // Ignore shift.
            Key.Modifiers = (Key.Modifiers & ~PlatformKeyModifier_Shift);
            
            if(Key.Modifiers & PlatformKeyModifier_Alt && Key.Codepoint == 'b')
            {
                DebugBreak();
            }
            
            if(Key.Modifiers == PlatformKeyModifier_Control)
            {
                if(0) {}
                else if(Key.Codepoint == 'h')
                {
                    DeleteChar(App);
                }
                else if(Key.Codepoint == 's')
                {
                    str8 File = PushS8(FrameArena, App->TextCount);
                    for EachIndex(Idx, App->TextCount)
                    {
                        File.Data[Idx] = (u8)App->Text[Idx];
                    }
                    
                    char *OutPath = PathFromExe(FrameArena, Memory->ExeDirPath, S8("file.c"));
                    OS_WriteEntireFile(OutPath, File);
                    
                    Log("Saved.\n");
                }
                else if(Key.Codepoint == '+')
                {
                    App->HeightPx += 1.f;
                }
                else if(Key.Codepoint == '-')
                {
                    App->HeightPx -= 1.f;
                }
                else if(Key.Codepoint == '0')
                {
                    App->HeightPx = DefaultHeightPx;
                }
            }
            
            if(Key.Modifiers == PlatformKeyModifier_None)
            {
                AppendChar(App, Key.Codepoint);
            }
        }
        else
        {
            if(0) {}
            else if(Key.Codepoint == PlatformKey_BackSpace)
            {
                DeleteChar(App);
            }
            else if(Key.Codepoint == PlatformKey_Return)
            {
                AppendChar(App, L'\n');
            }
            else if(Key.Codepoint == PlatformKey_Escape)
            {
                ShouldQuit = true;
            }
        }
    }
    
    gl_handle VAOs[2] = {};
    gl_handle VBOs[3] = {};
    gl_handle Textures[2] = {};
    gl_handle SoftwareShader, RectShader;
    glGenVertexArrays(ArrayCount(VAOs), &VAOs[0]);
    glGenBuffers(ArrayCount(VBOs), &VBOs[0]);
    glGenTextures(ArrayCount(Textures), &Textures[0]);
    glBindVertexArray(VAOs[0]);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    
    v4 BackgroundColor = Color_Night3;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    
    glClearColor(BackgroundColor.X, BackgroundColor.Y, BackgroundColor.Z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    f32 WindowBorderSize = 2.f;
    v4 WindowBorderColor;
    
    if(0) {}
    else if(Input->PlatformIsRecording) WindowBorderColor = Color_Red;
    else if(Input->PlatformIsPlaying) WindowBorderColor = Color_Green;
    else if(Input->PlatformWindowIsFocused) WindowBorderColor = Color_Snow0;
    else WindowBorderColor = Color_Night3;
    
    //- Prepare rects rendering 
    u64 BufferMaxSize = KB(40)/sizeof(f32);
    f32 *RectsBufferData = PushArray(FrameArena, f32, BufferMaxSize);
    // TODO(luca): These should go in the vertex shader as a constant.
    f32 QuadPosData[] =
    {
        -1.f, +1.f,
        +1.f, +1.f,
        -1.f, -1.f,
        +1.f, -1.f,
    };
    
    MemoryCopy(RectsBufferData, QuadPosData, sizeof(QuadPosData));
    GlobalRectsCount = 0;
    GlobalRectsInstances = (rect_instance *)(RectsBufferData + ArrayCount(QuadPosData));
    
    //- Draw text
    
    if(App->Font.Initialized)
    {
        if(App->PreviousHeightPx != App->HeightPx)
        {
            App->PreviousHeightPx = App->HeightPx;
            InitAtlas(App->FontAtlasArena, &App->Font, &App->FontAtlas, App->HeightPx);
        }
    }
    
    v2 BufferDim = V2S32(Buffer->Width, Buffer->Height);
    
    // Draw rectangles 
    {
        f32 Time = (f32)OS_GetWallClock();
        // Window borders
        {
            rect Dest = RectFromSize(V2(0.f, 0.f), BufferDim);
            rect_instance *Inst = DrawRect(Dest, WindowBorderColor, 0.f, WindowBorderSize, 0.f);
            Inst->Color0 = WindowBorderColor;
            Inst->Color1.W = 0.2f;
            Inst->Color2.W = 0.2f;
            Inst->Color3 = WindowBorderColor;
        }
    }
    
    // TODO(luca): Titlebar
    // Open | Save                                                   Close
    
    ui_box *Root = PushStructZero(FrameArena, ui_box);
    
    Root->FixedPosition = V2(0.f, 0.f);
    Root->FixedSize = BufferDim;
    Root->FixedPosition = V2AddF32(Root->FixedPosition, WindowBorderSize);
    Root->FixedSize = V2SubF32(Root->FixedSize, 2.f*WindowBorderSize);
    
    UI_Root = Root;
    UI_Current = Root;
    UI_Input = Input;
    UI_BoxTableSize = App->UIBoxTableSize;
    UI_BoxTable = App->UIBoxTable;
    UI_BoxArena = App->UIArena;
    UI_Atlas = &App->FontAtlas;
    
    // TODO(luca): 
    //1. Have a hash table
    //2. Have a frame index
    //3. If box wasn't touched in this iteration, evict from hash table?
    //4. You can get the previous size and position from this hash table.
    
    Root->Parent = Root->First = Root->Next = Root->Last = UI_NilBox;
    
    ui_size Size[Axis2_Count] = {};
    
    u32 Flags = (UI_BoxFlag_DrawBorders | UI_BoxFlag_DrawBackground |
                 UI_BoxFlag_DrawDisplayString |
                 UI_BoxFlag_CenterTextHorizontally | UI_BoxFlag_CenterTextVertically );
    
    UI_PushBox();
    Size[Axis2_X] = {UI_SizeKind_PercentOfParent, 1.f, 1.f};
    Size[Axis2_Y] = {UI_SizeKind_TextContent, 50.f, 1.f};
    UI_AddBox(S8("titlebar"), (Flags ^ 
                               UI_BoxFlag_DrawDisplayString ^ 
                               UI_BoxFlag_DrawBackground), Size);
    UI_PushBox();
    
    Size[Axis2_X] = {UI_SizeKind_Pixels, 80.f, 1.f}; 
    Size[Axis2_X] = {UI_SizeKind_PercentOfParent, 1.0f/3.f, 1.f}; 
    
    Size[Axis2_Y] = {UI_SizeKind_TextContent, 50.f, 1.f}; 
    u32 ButtonFlags = (Flags | 
                       UI_BoxFlag_AnimateColorOnHover |
                       UI_BoxFlag_AnimateColorOnPress);
    UI_AddBox(S8("Open"), ButtonFlags, Size);
    UI_AddBox(S8("Save"), ButtonFlags, Size);
    // TODO(luca): Spacer
    if(UI_AddBox(S8("Close"), ButtonFlags, Size)->Clicked)
    {
        ShouldQuit = true;
    }
    UI_PopBox();
    
    // TODO(luca): 
    //UI_SetNextAxis(Axis2_Y);
    //UI_AddBox(S8("Textarea"), UI_BoxFlag_DrawBorders, Size);
    
    UI_DrawBoxes(Root);
    
    //- Rendering 
    
    // Render rectangles
    {
        glBindVertexArray(VAOs[0]);
        RectShader = gl_ProgramFromShaders(FrameArena, Memory->ExeDirPath,
                                           S8("../source/shaders/rect_vert.glsl"),
                                           S8("../source/shaders/rect_frag.glsl"));
        glUseProgram(RectShader);
        
        // Uniforms
        {
            gl_handle UViewport = glGetUniformLocation(RectShader, "Viewport");
            glUniform2f(UViewport, V2Arg(BufferDim));
            
            gl_LoadTextureFromImage(Textures[0], Atlas->Width, Atlas->Height, Atlas->Data, GL_RED, RectShader, "Texture");
            
            
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
        
        u64 Size = sizeof(QuadPosData) + GlobalRectsCount*sizeof(rect_instance);
        glBufferData(GL_ARRAY_BUFFER, Size, RectsBufferData, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribDivisor(0, 0);  
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2*sizeof(f32), 0);
        
        u64 Offset = ArrayCount(QuadPosData);
        
        // TODO(luca): Metaprogram
        s32 Counts[] = {4,4, 4,4,4,4, 4, 1,1, 1};
        
        s32 TotalSize = 0;
        for EachElement(Idx, Counts)
        {            
            gl_SetQuadAttribute((s32)Idx + 1, Counts[Idx], &Offset);
            TotalSize += Counts[Idx];
        }
        Assert(TotalSize == (sizeof(rect_instance)/sizeof(f32)));
        
        glEnable(GL_MULTISAMPLE);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GlobalRectsCount);
    }
    
    // Cleanup
    {
        glDeleteVertexArrays(ArrayCount(VAOs), &VAOs[0]);
        glDeleteTextures(ArrayCount(Textures), &Textures[0]);
        glDeleteBuffers(ArrayCount(VBOs), &VBOs[0]);
        glDeleteProgram(RectShader);
    }
    
    return ShouldQuit;
}
