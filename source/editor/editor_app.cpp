#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"
#include "editor_math.h"
#include "editor_platform.h"
#include "editor_font.h"
#include "editor_random.h"
#include "editor_libs.h"
#include "editor_gl.h"
#include "editor_app.h"

#define U32ToV3Arg(Hex) \
((f32)((Hex >> 8*2) & 0xFF)/255.0f), \
((f32)((Hex >> 8*1) & 0xFF)/255.0f), \
((f32)((Hex >> 8*0) & 0xFF)/255.0f)

#define ColorList \
SET(Text,             0xff87bfcf) \
SET(Point,            0xff00ffff) \
SET(Cursor,           0xffff0000) \
SET(Button,           0xff0172ad) \
SET(ButtonHovered,    0xff017fc0) \
SET(ButtonPressed,    0xff0987c8) \
SET(ButtonText,       0xfffbfdfe) \
SET(Foreground,       0xffd8dee9) \
SET(Background,       0xff13171f) \
SET(BackgroundSecond, 0xff3a4151) \
SET(Red,              0xffbf616a) \
SET(Green,            0xffa3be8c) \
SET(Orange,           0xffd08770) \
SET(Magenta,          0xffb48ead) \
SET(Yellow,           0xffebcb8b)

#define SET(Name, Value) u32 ColorU32_##Name = Value;
ColorList
#undef SET

#define SET(Name, Value) v3 Color_##Name = {U32ToV3Arg(Value)};
ColorList
#undef SET

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
    
    for EachIndexType(s32, Idx, Atlas->CodepointsCount)
    {
        float UnusedX, UnusedY;
        stbtt_GetPackedQuad(Atlas->PackedChars, Atlas->Width, Atlas->Height, 
                            Idx,
                            &UnusedX, &UnusedY,
                            &Atlas->AlignedQuads[Idx], 0);
    }
    
}

void
DrawChar(font_atlas *Atlas, v2 *Positions, v2 *TexCoords, s32 *VerticesCount, 
         v2 Pos, rune Codepoint)
{
    rune CharIdx = Codepoint - Atlas->FirstCodepoint;
    
    v2 *Position = Positions + *VerticesCount;
    v2 *TexCoord = TexCoords + *VerticesCount;
    *VerticesCount += 6;
    
    stbtt_packedchar *PackedChar = &Atlas->PackedChars[CharIdx];
    f32 Width = (PackedChar->x1 - PackedChar->x0)*Atlas->PixelScaleWidth;
    f32 Height = (PackedChar->y1 - PackedChar->y0)*Atlas->PixelScaleHeight;
    {
        v2 Min = Pos;
        Min.X += (PackedChar->xoff)*Atlas->PixelScaleWidth;
        Min.Y -= (PackedChar->yoff)*Atlas->PixelScaleHeight;
        v2 Max = V2(Min.X + Width, Min.Y - Height);
        MakeQuadV2(Position, Min, Max);
    }
    
    stbtt_aligned_quad *Quad = &Atlas->AlignedQuads[CharIdx]; // '!'
    MakeQuadV2(TexCoord, V2(Quad->s0, Quad->t0), V2(Quad->s1, Quad->t1));
}

void
DrawRect(v4 Dest, v4 Color, f32 CornerRadius, f32 BorderThickness, f32 Softness)
{
    rect_quad_data *Data = GlobalRectQuadData + GlobalRectsCount;
    GlobalRectsCount += 1;
    
    Data->Dest = Dest;
    Data->Color = Color;
    Data->CornerRadius = CornerRadius;
    Data->BorderThickness = BorderThickness;
    Data->Softness = Softness;
}

C_LINKAGE
UPDATE_AND_RENDER(UpdateAndRender)
{
    b32 ShouldQuit = false;
    
#if EDITOR_INTERNAL
    GlobalDebuggerIsAttached = Memory->IsDebuggerAttached;
#endif
    
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
        
        Memory->Initialized = true;
    }
    
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
                DebugBreak;
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
                    App->HeightPx++;
                }
                else if(Key.Codepoint == '-')
                {
                    App->HeightPx--;
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
    
    gl_handle VAOs[1] = {};
    gl_handle VBOs[2] = {};
    gl_handle Textures[1] = {};
    gl_handle SoftwareShader, TextShader, RectShader;
    glGenVertexArrays(ArrayCount(VAOs), &VAOs[0]);
    glGenBuffers(ArrayCount(VBOs), &VBOs[0]);
    glGenTextures(ArrayCount(Textures), &Textures[0]);
    glBindVertexArray(VAOs[0]);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    
    v3 BackgroundColor = Color_BackgroundSecond;
    
    glClearColor(BackgroundColor.X, BackgroundColor.Y, BackgroundColor.Z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    s32 WindowBorderSize = 2;
    v3 WindowBorderColor;
    
    if(0) {}
    else if(Input->PlatformIsRecording) WindowBorderColor = Color_Red;
    else if(Input->PlatformIsPlaying) WindowBorderColor = Color_Green;
    else if(Input->PlatformWindowIsFocused) WindowBorderColor = Color_Foreground;
    else WindowBorderColor = Color_Background;
    
    //- Prepare rects rendering 
    u64 BufferMaxSize = KB(1)/sizeof(f32);
    f32 *RectsBufferData = PushArray(FrameArena, f32, BufferMaxSize);
    f32 QuadPosData[] =
    {
        -1.f, +1.f,
        +1.f, +1.f,
        -1.f, -1.f,
        +1.f, +1.f,
        -1.f, -1.f,
        +1.f, -1.f,
    };
    
    MemoryCopy(RectsBufferData, QuadPosData, sizeof(QuadPosData));
    GlobalRectsCount = 0;
    GlobalRectQuadData = (rect_quad_data *)(RectsBufferData + ArrayCount(QuadPosData));
    
    //- Render text (rasterized on CPU)
    app_offscreen_buffer TextImage;
    {    
        TextImage.Width = Buffer->Width - WindowBorderSize;
        TextImage.Height = Buffer->Height - WindowBorderSize;
        TextImage.BytesPerPixel = Buffer->BytesPerPixel;
        TextImage.Pitch = TextImage.BytesPerPixel*TextImage.Width;
        u64 Size = TextImage.BytesPerPixel*(TextImage.Width*TextImage.Height);
        TextImage.Pixels = PushArray(FrameArena, u8, Size);
        MemorySet(TextImage.Pixels, 0, Size);
    }
    
    v2 *TextTexCoords = 0;
    v2 *TextPositions = 0;
    s32 TextVerticesCount = 0;
    
    if(App->Font.Initialized)
    {
        s32 QuadCount = 6;
        s32 MaxVerticesCount = QuadCount*1024;
        TextPositions = PushArray(FrameArena, v2, MaxVerticesCount);
        TextTexCoords = PushArray(FrameArena, v2, MaxVerticesCount);
        
        if(App->PreviousHeightPx != App->HeightPx)
        {
            App->PreviousHeightPx = App->HeightPx;
            InitAtlas(App->FontAtlasArena, &App->Font, &App->FontAtlas, App->HeightPx);
        }
        
        // NOTE(luca): Should these be minus border size?
        Atlas->PixelScaleHeight = (2.0f / (f32)(Buffer->Height));
        Atlas->PixelScaleWidth  = (2.0f / (f32)(Buffer->Width));
        
        f32 StartX = ((f32)WindowBorderSize*Atlas->PixelScaleWidth + -1.0f);
        
        str8 Text = {.Data = (u8 *)App->Text, .Size = App->TextCount};
        
        f32 MaxWidth = -StartX + 1.0f;
        f32 CumulatedWidth = 0;
        u32 StartIdx = 0;
        // NOTE(luca): Monospace, all widths will be the same.
        f32 CharWidth = (Atlas->PackedChars[0].xadvance)*Atlas->PixelScaleWidth;
        f32 CharHeight = (Atlas->PixelScaleHeight*App->HeightPx);
        
        v2 Offset = V2(StartX, (1.0f - CharHeight));
        
        for EachIndexType(u32, Idx, App->TextCount)
        {
            CumulatedWidth += CharWidth;
            
            if(App->Text[Idx] == L'\n')
            {
                CumulatedWidth = 0.0f;
                // Wrap()
                {
                    Offset.X = StartX;
                    Offset.Y -= CharHeight;
                }
                
                StartIdx = Idx + 1;
            }
            else if(CumulatedWidth > MaxWidth)
            {
                // TODO(luca): We should search backwards for a whitespace instead.
                CumulatedWidth = 0;
                
                // Wrap()
                {
                    Offset.X = StartX;
                    Offset.Y -= CharHeight;
                }
                
                // We go back one to draw the character we wrapped on.
                StartIdx = Idx;
                Idx -= 1;
            }
            else
            {
                DrawChar(Atlas, TextPositions, TextTexCoords, &TextVerticesCount, Offset, App->Text[Idx]);
                Offset.X += CharWidth;
            }
        }
        
        v2 Pos = Offset;
        DrawChar(Atlas, TextPositions, TextTexCoords, &TextVerticesCount, Pos, L'_');
    }
    
    // Draw rectangles 
    {    
        rect ButtonDim;
        ButtonDim.Min = V2(480.f, 110.f);
        ButtonDim.Max = V2(870.f, 310.f);
        v3 ButtonFillColor = Color_Button;
        f32 ButtonCornerRadius = 10.f;
        f32 ButtonSoftness = 1.f;
        v3 ButtonBorderColor = {};
        f32 ButtonBorderThickness = 3.f;
        
        v2 MousePos = V2S32(Input->MouseX, Input->MouseY);
        if(IsInsideRectV2(MousePos, ButtonDim))
        {
            if(Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
            {
                ButtonFillColor = Color_ButtonPressed;
            }
            else
            {        
                ButtonFillColor = Color_ButtonHovered;
            }
        }
        
        DrawRect(V4(80.f,  80.f,  280.f, 280.f), V4(0.f, 0.f, 1.f, 1.f), 10.f, 1.f, 1.f);
        DrawRect(V4(200.f, 320.f, 340.f, 420.f), V4(1.f, 0.f, 1.f, 1.f), 20.f, 0.f, 1.f);
        DrawRect(V4(200.f, 320.f, 340.f, 420.f), V4(0.f, 0.f, 0.f, 1.f), 20.f, 3.f, 1.f);
        DrawRect(V4(RectArg(ButtonDim)), V4(V3Arg(ButtonFillColor), 1.f), 
                 ButtonCornerRadius, 0.f, ButtonSoftness);
        DrawRect(V4(RectArg(ButtonDim)), V4(V3Arg(ButtonBorderColor), 1.f), 
                 ButtonCornerRadius, ButtonBorderThickness, ButtonSoftness);
        DrawRect(V4(0.f, 0.f, (f32)Buffer->Width, (f32)Buffer->Height), V4(V3Arg(WindowBorderColor), 1.f),
                 0.f, (f32)WindowBorderSize, 0.f);
        
    }
    
    //- Rendering 
    
    // Render text
    {
        TextShader = gl_ProgramFromShaders(FrameArena, Memory->ExeDirPath,
                                           S8("../source/shaders/text_vert.glsl"),
                                           S8("../source/shaders/text_frag.glsl"));
        glUseProgram(TextShader);
        
        gl_LoadTextureFromImage(Textures[0], Atlas->Width, Atlas->Height, Atlas->Data, GL_RED, TextShader);
        gl_LoadFloatsIntoBuffer(VBOs[0], TextShader, "pos", TextVerticesCount, 2, TextPositions);
        gl_LoadFloatsIntoBuffer(VBOs[1], TextShader, "tex", TextVerticesCount, 2, TextTexCoords);
        gl_handle UTextColor = glGetUniformLocation(TextShader, "TextColor"); 
        glUniform4f(UTextColor, V3Arg(Color_Text), 1.0f);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glDisable(GL_DEPTH_TEST);
        
        glDrawArrays(GL_TRIANGLES, 0, TextVerticesCount);
    }
    
    // Render rectangles
    {
        RectShader = gl_ProgramFromShaders(FrameArena, Memory->ExeDirPath,
                                           S8("../source/shaders/rect_vert.glsl"),
                                           S8("../source/shaders/rect_frag.glsl"));
        glUseProgram(RectShader);
        
        gl_handle UViewport = glGetUniformLocation(RectShader, "Viewport");
        glUniform2f(UViewport, (f32)(Buffer->Width), (f32)(Buffer->Height));
        
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        
        u64 Size = sizeof(QuadPosData) + GlobalRectsCount*sizeof(rect_quad_data);
        glBufferData(GL_ARRAY_BUFFER, Size, RectsBufferData, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribDivisor(0, 0);  
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2*sizeof(f32), 0);
        
        u64 Offset = ArrayCount(QuadPosData);
        gl_SetQuadAttribute(1, 4, &Offset);
        gl_SetQuadAttribute(2, 4, &Offset);
        gl_SetQuadAttribute(3, 1, &Offset);
        gl_SetQuadAttribute(4, 1, &Offset);
        gl_SetQuadAttribute(5, 1, &Offset);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GlobalRectsCount);
    }
    
    // Cleanup
    {
        glDeleteVertexArrays(ArrayCount(VAOs), &VAOs[0]);
        glDeleteTextures(ArrayCount(Textures), &Textures[0]);
        glDeleteBuffers(ArrayCount(VBOs), &VBOs[0]);
        glDeleteProgram(TextShader);
        glDeleteProgram(RectShader);
    }
    
    return ShouldQuit;
}
