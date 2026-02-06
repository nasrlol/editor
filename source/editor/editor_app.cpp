#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"
#include "editor_platform.h"
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
            else if(Key.Codepoint == 'b')
            {
                DebugBreak;
            }
        }
        else
        {
            if(Key.Codepoint == PlatformKey_Escape)
            {
                ShouldQuit = true;
            }
        }
    }
    
    gl_handle VAOs[1] = {};
    gl_handle VBOs[2] = {};
    gl_handle Textures[1] = {};
    glGenVertexArrays(ArrayCount(VAOs), &VAOs[0]);
    glGenBuffers(ArrayCount(VBOs), &VBOs[0]);
    glGenTextures(ArrayCount(Textures), &Textures[0]);
    glBindVertexArray(VAOs[0]);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    glClearColor(U32ToV3Arg(ColorU32_BackgroundSecond), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Cleanup
    {    
        glDeleteTextures(ArrayCount(Textures), &Textures[0]);
        glDeleteBuffers(ArrayCount(VBOs), &VBOs[0]);
    }
    
    return ShouldQuit;
}
