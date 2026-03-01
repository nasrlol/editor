#define RL_BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"

#include "ex_random.h"
#include "ex_math.h"
#include "ex_platform.h"
#include "ex_libs.h"
#include "ex_gl.h"
#include "ex_app.h"

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

#define Path_Code                 ".." SLASH "code" SLASH "example" SLASH 

internal app_offscreen_buffer 
LoadImage(arena *Arena, str8 ExeDirPath, str8 Path)
{
    app_offscreen_buffer Result = {};
    
    char *FilePath = PathFromExe(Arena, ExeDirPath, Path);
    str8 File = OS_ReadEntireFileIntoMemory(FilePath);
    s32 Width, Height, Components;
    s32 BytesPerPixel = 4;
    u8 *Image = stbi_load_from_memory(File.Data, (int)File.Size, &Width, &Height, &Components, BytesPerPixel);
    Assert(Components == BytesPerPixel);
    
    Result.Width = Width;
    Result.Height = Height;
    Result.BytesPerPixel = BytesPerPixel;
    Result.Pitch = Result.BytesPerPixel*Width;
    Result.Pixels = Image;
    
    return Result;
}

#define AssetPath(FileName) S8("../data/game/" FileName)

internal entity *
PushEntities(app_state *App, u64 Count)
{
    entity *Result = PushArray(App->EntitiesArena, entity, Count);
    App->EntitiesCount += Count;
    return Result;
}

internal void
FreeEntities(app_state *App, u64 Count)
{
    App->EntitiesArena->Pos -= sizeof(entity)*Count;
}

internal void 
InitEntities(app_state *App, v2 EntityDim)
{
    arena *Arena = App->EntitiesArena;
    
    App->EntitiesCount = 0;
    FreeEntities(App, App->EntitiesCount);
    
    App->Entities = PushEntities(App, 1);
    App->Player = App->Entities + 0;
    App->Player->P = V2(0.0f, -0.7f);
    
    App->EnemiesCount = 22;
    App->Enemies = PushEntities(App, App->EnemiesCount);
    App->EnemiesDeadCount = 0;
    
    App->EnemiesPerRow = 11;
    
    v2 EnemyBaseP;
    EnemyBaseP.X = EntityDim.X/2.0f + (0.0f - (EntityDim.X*(f32)App->EnemiesPerRow))/2.0f;
    EnemyBaseP.Y = 1.0f - EntityDim.Y/2.0f;
    v2 Pad = V2(0.0f, 0.0f);
    for EachIndex(Idx, App->EnemiesCount)
    {
        entity *Enemy = App->Enemies + Idx;
        Enemy->P.X = EnemyBaseP.X + ((f32)(Idx%App->EnemiesPerRow) * (EntityDim.X + Pad.X));
        Enemy->P.Y = EnemyBaseP.Y - ((f32)(Idx/App->EnemiesPerRow) * (EntityDim.Y + Pad.Y));
    }
}

C_LINKAGE
UPDATE_AND_RENDER(UpdateAndRender)
{
    b32 ShouldQuit = false;
    
#if RL_INTERNAL    
    GlobalDebuggerIsAttached = Memory->IsDebuggerAttached;
#endif
    
    local_persist s32 GLADVersion = gladLoaderLoadGL();
    
    v2 EntityDim = V2(0.10f, 0.10f);
    
    if(!Memory->Initialized)
    {
        Memory->AppState = PushStruct(PermanentArena, app_state);
        app_state *App = (app_state *)Memory->AppState;
        App->EntitiesArena = ArenaAlloc();
        
        InitEntities(App, EntityDim);
        
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
            else if(Key.Codepoint == 'r')
            {
                InitEntities(App, EntityDim);
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
    
    f32 ddP = 0.0f;
    
    // Game input
    {
        if(WasPressed(Input->ActionDown))
        {
            entity *FoundEntity = 0;
            
            for EachIndex(Idx, App->EnemiesCount)
            {
                entity *Enemy = App->Enemies + Idx;
                
                if(!Enemy->IsDead)
                {                    
                    rect EnemyBounds = RectFromCenterDim(Enemy->P, EntityDim);
                    
                    // NOTE(luca): Due to floating point rounding if another entity is inside it might get prioritized so we need an epsilon value.  Probably we should calculate the distance to each entity in bounds and keep the one with the closest distance so that we can handle entities that are inside eachother, but for now, we do not allow that.
                    f32 tEpsilon = 0.0001f;
                    
                    if(App->Player->P.X > EnemyBounds.Min.X + tEpsilon && 
                       App->Player->P.X < EnemyBounds.Max.X - tEpsilon)
                    {
                        // NOTE(luca): Floating point rounding might make other enemies collide.
                        FoundEntity = Enemy;
                        // NOTE(luca): Don't break, take the last one since that should be the one nearest to the player.
                    }
                }
            }
            
            if(FoundEntity)
            {
                FoundEntity->IsDead = true;
                App->EnemiesDeadCount += 1;
            }
        }
        
        if(Input->MoveLeft.EndedDown)
        {
            ddP += -1.0f;
        }
        
        if(Input->MoveRight.EndedDown)
        {
            ddP += 1.0f;
        }
    }
    
    // Move the player
    {
        f32 Speed = 30.0f;
        
        ddP *= Speed;
        
        f32 dt = Input->dtForFrame;
        f32 dP = App->PlayerdP;
        
        ddP += -8.0f*dP;
        
        // Equations of motion integrated
        // f(t) = 0.5*a*(t^2) + v*t + p
        // f'(t) = a*t + v
        // f''(t) = a
        
        f32 PlayerDelta = ((0.5f*ddP*Square(dt)) + (dP*dt));
        
        App->PlayerdP = ddP*dt + dP;
        App->Player->P.X += PlayerDelta;
        
        f32 HalfWidth = EntityDim.X*0.5f;
        // Handle collisions, walls are the edge of the screen
        {        
            f32 MinX = (-1.0f + HalfWidth);
            f32 MaxX = (1.0f - HalfWidth);
            if(App->Player->P.X < MinX)
            {
                App->PlayerdP = 0.0f;
                App->Player->P.X = MinX;
            }
            
            if(App->Player->P.X > MaxX)
            {
                App->PlayerdP = 0.0f;
                App->Player->P.X = MaxX;
            }
        }
        
    }
    
    local_persist app_offscreen_buffer EnemyImage = LoadImage(PermanentArena, Memory->ExeDirPath, AssetPath("enemy.png"));
    
    gl_handle VAOs[1] = {};
    gl_handle VBOs[2] = {};
    gl_handle Textures[1] = {};
    glGenVertexArrays(ArrayCount(VAOs), &VAOs[0]);
    glGenBuffers(ArrayCount(VBOs), &VBOs[0]);
    glGenTextures(ArrayCount(Textures), &Textures[0]);
    glBindVertexArray(VAOs[0]);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(App->EnemiesDeadCount != App->EnemiesCount)
    {
        glClearColor(V3Arg(Color_BackgroundSecond), 0.0f);
    }
    else
    {
        glClearColor(V3Arg(Color_Background), 0.0f);
    }
    
    gl_handle Shader;
    
    // Render entities
    s32 MaxVerticesCount = App->EntitiesCount*6;
    v3 *Vertices = PushArray(FrameArena, v3, MaxVerticesCount);
    v2 *TexCoords = PushArray(FrameArena, v2, MaxVerticesCount);
    Shader = gl_ProgramFromShaders(FrameArena, Memory->ExeDirPath, 
                                   S8(Path_Code "vert_text.glsl"),
                                   S8(Path_Code "frag_text.glsl"));
    glUseProgram(Shader);
    
    gl_LoadTextureFromImage(Textures[0], 
                            EnemyImage.Width, EnemyImage.Height, EnemyImage.Pixels,
                            GL_RGBA, Shader);
    
    s32 VerticesCount = 0;
    for EachIndex(Idx, App->EntitiesCount)
    {
        entity *Entity = App->Entities + Idx;
        
        if(!Entity->IsDead)
        {        
            MakeQuadV2(TexCoords + VerticesCount, V2(0.0f, 1.0f), V2(1.0f, 0.0f));
            
            v2 Min; V2Math { Min.E = Entity->P.E - EntityDim.E/2.0f; }
            v2 Max = V2AddV2(Min, EntityDim);
            MakeQuadV3(Vertices + VerticesCount, Min, Max, -1.0f);
            
            VerticesCount += 6;
        }
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    gl_LoadFloatsIntoBuffer(VBOs[0], Shader, "pos", VerticesCount, 3, Vertices);
    gl_LoadFloatsIntoBuffer(VBOs[1], Shader, "tex", VerticesCount, 2, TexCoords);
    
    glDrawArrays(GL_TRIANGLES, 0, VerticesCount);
    
    // Cleanup
    {    
        glDeleteTextures(ArrayCount(Textures), &Textures[0]);
        glDeleteBuffers(ArrayCount(VBOs), &VBOs[0]);
        glDeleteProgram(Shader);
    }
    
    return ShouldQuit;
}