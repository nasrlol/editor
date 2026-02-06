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
    App->Text[App->TextCount] = Codepoint;
    App->TextCount += 1;
    Assert(App->TextCount < ArrayCount(App->Text));
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
            if(0) {}
            else if(Key.Modifiers & PlatformKeyModifier_Alt && Key.Codepoint == 'b')
            {
                DebugBreak;
            }
            else if(Key.Modifiers == PlatformKeyModifier_Control &&
                    Key.Codepoint == 'h')
            {
                if(App->TextCount) App->TextCount -= 1;
            }
            else if(Key.Modifiers == PlatformKeyModifier_None || 
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
                if(App->TextCount) App->TextCount -= 1;
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
        
        f32 HeightPx = 24.0f;;
        f32 FontScale = stbtt_ScaleForPixelHeight(&App->Font.Info, HeightPx);
        f32 Baseline = GetBaseLine(&App->Font, FontScale);
        str8 Text = {.Data = (u8 *)App->Text, .Size = App->TextCount};
        
#if 1        
        s32 *CharacterPixelWidths = PushArray(FrameArena, s32, App->TextCount);
        u64 MaxWrapPositionsCount = App->TextCount;
        u32 *WrapPositions = PushArray(FrameArena, u32, MaxWrapPositionsCount);
        u32 WrapPositionsCount = 0;
        
        // 1. First pass where we save characters' widths.
        // 2. Find positions where we need to wrap.
        // Wrapping algorithm
        // 1. When a character exceeds the box maximum width search backwards for whitespace.
        //    I. Whitespace found?
        //       Y -> Length of string until whitespace would fit?
        //            Y -> Save whitespace's position.  This becomes the new searching start position.
        //            N -> Break on the character that exceeds the maximum width.
        //       N -> Break on the character that exceeds the maximum width.
        //    II. Continue until end of string.
        
        // Save widths 
        for EachIndex(Idx, App->TextCount)
        {
            rune CharAt = App->Text[Idx];
            
            s32 AdvanceWidth, LeftSideBearing;
            stbtt_GetCodepointHMetrics(&App->Font.Info, CharAt, &AdvanceWidth, &LeftSideBearing);
            
            CharacterPixelWidths[Idx] = (s32)roundf(FontScale*(f32)AdvanceWidth);
        }
        
        s32 MaxWidth = Buffer->Width;
        Assert(MaxWidth >= 0);
        
        // Find positions 
        u32 SearchStart = 0;
        while(SearchStart < App->TextCount)
        {
            s32 CumulatedWidth = 0;
            u32 SearchIndex = SearchStart;
            for(;
                ((SearchIndex < App->TextCount) &&
                 (CumulatedWidth <= MaxWidth));
                SearchIndex++)
            {
                s32 Width = CharacterPixelWidths[SearchIndex];
                CumulatedWidth += Width;
                
                if(App->Text[SearchIndex] == '\n')
                {
                    WrapPositions[WrapPositionsCount] = SearchIndex;
                    WrapPositionsCount += 1;
                    SearchStart = SearchIndex + 1;
                    break;
                }
                
            }
            
            if(SearchIndex < App->TextCount && 
               App->Text[SearchIndex] == '\n') continue;
            
            if(CumulatedWidth > MaxWidth)
            {
                // Search backwards for a space.
                SearchIndex--;
                u32 SearchIndexStop = SearchIndex;
                
                while(SearchIndex > SearchStart)
                {
                    if(App->Text[SearchIndex] == ' ')
                    {
                        WrapPositions[WrapPositionsCount] = SearchIndex;
                        WrapPositionsCount += 1;
                        break;
                    }
                    
                    SearchIndex--;
                }
                
                if(SearchIndex > SearchStart)
                {
                    // NOTE(luca): We have wrapped.
                    Assert(SearchIndex > SearchStart);
                    SearchStart = SearchIndex + 1;
                }
                else if(SearchIndex == SearchStart)
                {
                    // NOTE(luca): We have to wrap on the last character even though it isn't a space.
                    Assert(SearchIndexStop > SearchStart);
                    WrapPositions[WrapPositionsCount] = SearchIndex;
                    WrapPositionsCount += 1;
                    SearchStart = SearchIndexStop;
                }
                else
                {
                    InvalidPath;
                }
                
            }
            else
            {
                // NOTE(luca): We don't need to wrap, we've reached the end of the text.
                break;
            }
        }
#endif
        
        u32 LastStart = 0;
        for EachIndex(Idx, WrapPositionsCount + 1)
        {
            u32 Start = LastStart;
            
            u32 End = WrapPositions[Idx];
            if(Idx == WrapPositionsCount)
            {
                End = (u32)App->TextCount;
            }
            
            str8 Line = {.Data = (u8 *)(App->Text + Start), .Size = (End - Start)};
            v2 TextPos = V2(0.0f, Baseline*(1.0f + (f32)Idx));
            DrawText(FrameArena, Buffer, &App->Font, HeightPx, TextPos, true, ColorU32_Text, Line);
            
            LastStart = End + 1;
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