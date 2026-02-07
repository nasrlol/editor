#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"
#include "editor_platform.h"
#include "editor_font.h"
#include "editor_random.h"
#include "editor_math.h"
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

C_LINKAGE
UPDATE_AND_RENDER(UpdateAndRender)
{
    
    b32 ShouldQuit = false;
    
#if EDITOR_INTERNAL
    GlobalDebuggerIsAttached = Memory->IsDebuggerAttached;
#endif
    
    local_persist s32 GLADVersion = gladLoaderLoadGL();
    
    if(!Memory->Initialized)
    {
        Memory->AppState = PushStruct(PermanentArena, app_state);
        app_state *App = (app_state *)Memory->AppState;
        
        char *FontPath = PathFromExe(FrameArena, Memory->ExeDirPath, S8("../data/font_regular.ttf"));
        InitFont(&App->Font, FontPath);
        
        Memory->Initialized = true;
    }
    
    app_state *App = (app_state *)Memory->AppState;
    
    // Text input
    for EachIndex(Idx, Input->Text.Count)
    {
        app_text_button Key = Input->Text.Buffer[Idx];
        
        if(!Key.IsSymbol)
        {
            if(Key.Modifiers & PlatformKeyModifier_Alt && Key.Codepoint == 'b')
            {
                DebugBreak;
            }
            if(Key.Modifiers == PlatformKeyModifier_Control ||
               (Key.Modifiers == (PlatformKeyModifier_Control | PlatformKeyModifier_Shift)))
            {
                if(Key.Codepoint == 'h')
                {
                    DeleteChar(App);
                }
                if(Key.Codepoint == 's')
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
                if (Key.Codepoint == '+')
                {
                    HeightPx++;
                }
                
                if (Key.Codepoint == '-')
                {
                    HeightPx--;
                }
                if(Key.Codepoint == '0')
                {
                    HeightPx = DefaultHeightPx;
                }
            }
            if(Key.Modifiers == PlatformKeyModifier_None ||
               Key.Modifiers == PlatformKeyModifier_Shift)
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
    gl_handle TextShader;
    glGenVertexArrays(ArrayCount(VAOs), &VAOs[0]);
    glGenBuffers(ArrayCount(VBOs), &VBOs[0]);
    glGenTextures(ArrayCount(Textures), &Textures[0]);
    glBindVertexArray(VAOs[0]);
    TextShader = gl_ProgramFromShaders(FrameArena, Memory->ExeDirPath,
                                       S8("../source/shaders/text_vert.glsl"),
                                       S8("../source/shaders/text_frag.glsl"));
    glUseProgram(TextShader);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    glClearColor(U32ToV3Arg(ColorU32_BackgroundSecond), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render text (rasterized on CPU)
    {
        // TODO(luca): Use font atlas + cache for rendering on the GPU.
        MemorySet(Buffer->Pixels, 0, Buffer->Pitch*Buffer->Height);
        
        f32 FontScale = stbtt_ScaleForPixelHeight(&App->Font.Info, HeightPx);
        f32 Baseline = GetBaseLine(&App->Font, FontScale);
        s32 AdvanceWidth, LeftSideBearing;
        stbtt_GetCodepointHMetrics(&App->Font.Info, ' ', &AdvanceWidth, &LeftSideBearing);
        // NOTE(luca): Since we work on monospace fonts, we can assume all the characters will have th e same width.
        s32 CharWidth = (s32)roundf(FontScale*(f32)AdvanceWidth);
        
        v2 Offset = V2(0.0f, Baseline);
        str8 Text = {.Data = (u8 *)App->Text, .Size = App->TextCount};
        
        s32 MaxWidth = Buffer->Width;
        s32 CumulatedWidth = 0;
        u32 StartIdx = 0;
        for EachIndexType(u32, Idx, App->TextCount)
        {
            CumulatedWidth += CharWidth;
            
            if(App->Text[Idx] == L'\n')
            {
                CumulatedWidth = 0;
                Offset.X = 0.0f;
                Offset.Y += Baseline;
                StartIdx = Idx + 1;
            }
            else if(CumulatedWidth > MaxWidth)
            {
#if 0
                // TODO(luca): We should search backwards for a whitespace.
#else
                CumulatedWidth = 0;
                Offset.X = 0.0f;
                Offset.Y += Baseline;
                // We go back one to draw the character we wrapped on.
                StartIdx = Idx;
                Idx -= 1;
#endif
            }
            else
            {
                // Draw Character
                {
                    s32 AdvanceWidth, LeftSideBearing;
                    stbtt_GetCodepointHMetrics(&App->Font.Info, App->Text[Idx], &AdvanceWidth, &LeftSideBearing);
                    
                    s32 FontWidth, FontHeight;
                    s32 X0, Y0, X1, Y1;
                    u8 *FontBitmap = 0;
                    stbtt_GetCodepointBitmapBox(&App->Font.Info, App->Text[Idx],
                                                FontScale, FontScale,
                                                &X0, &Y0, &X1, &Y1);
                    FontWidth = (X1 - X0);
                    FontHeight = (Y1 - Y0);
                    FontBitmap = PushArray(FrameArena, u8, (FontWidth*FontHeight));
                    stbtt_MakeCodepointBitmap(&App->Font.Info, FontBitmap,
                                              FontWidth, FontHeight, FontWidth,
                                              FontScale, FontScale, App->Text[Idx]);
                    
                    s32 XOffset = (s32)floorf(Offset.X + (f32)LeftSideBearing*FontScale);
                    s32 YOffset = (s32)Offset.Y + Y0;
                    
                    DrawCharacter(FrameArena, Buffer, FontBitmap, FontWidth, FontHeight, XOffset, YOffset, ColorU32_Text);
                    
                    
                    Offset.X += roundf((f32)AdvanceWidth*FontScale);
                }
            }
        }
        
        // Draw cursor
        {        
            s32 MinX = (s32)Offset.X;
            s32 MinY = (s32)Offset.Y + 7;
            
            for(s32 Y = MinY - 25; Y < MinY; Y += 1)
            {
                for(s32 X = MinX; X < MinX + 11; X += 1)
                {
                    u32 *Pixel = (u32 *)(Buffer->Pixels + Buffer->BytesPerPixel*(Y*Buffer->Width + X));
                    *Pixel = ColorU32_Text;
                }
            }
        }
        
        s32 Count = 6;
        v3 *Vertices = PushArray(FrameArena, v3, Count);
        v2 *TexCoords = PushArray(FrameArena, v2, Count);
        
        MakeQuadV3(Vertices, V2(-1.0f, -1.0f), V2(1.0f, 1.0f), -1.0f);
        for EachIndex(Idx, Count)
        {
            V2Math
            {
                TexCoords[Idx].E = (Vertices[Idx].E + 1.0f)*0.5f;
            }
            TexCoords[Idx].Y = (1.0f - TexCoords[Idx].Y);
        }
        
        gl_LoadTextureFromImage(Textures[0], Buffer->Width, Buffer->Height, Buffer->Pixels, GL_RGBA,  TextShader);
        
        gl_LoadFloatsIntoBuffer(VBOs[0], TextShader, "pos", Count, 3, Vertices);
        gl_LoadFloatsIntoBuffer(VBOs[1], TextShader, "tex", Count, 2, TexCoords);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        
        glDrawArrays(GL_TRIANGLES, 0, Count);
    }
    
    // Cleanup
    {
        glDeleteTextures(ArrayCount(Textures), &Textures[0]);
        glDeleteBuffers(ArrayCount(VBOs), &VBOs[0]);
        glDeleteProgram(TextShader);
    }
    
    return ShouldQuit;
}
