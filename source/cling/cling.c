//~ Defines
#define BUILD ".." SLASH "build" SLASH

//~ Libaries
#define BASE_FORCE_THREADS_COUNT 1
#define BASE_CONSOLE_APPLICATION 1
#include "base/base.h"
#include "base/base.c"

#define CLING_CODE_PATH ".." SLASH "source" SLASH
#define CLING_SOURCE_PATH CLING_CODE_PATH "cling" SLASH "cling.c"
#include "cling/cling.h"

NO_WARNINGS_BEGIN
#include "lib/metadesk/md.h"
#include "lib/metadesk/md.c"
NO_WARNINGS_END

//~ Globals
static MD_Arena *GlobalMDArena = 0;

//~ Functions
internal void
RunCommand(void)
{
    str8 BuildCommand = Str8ArrayJoin(' ');
    Cng_RunCommand(BuildCommand);
}

internal void
WriteStreamToFile(MD_String8List Stream, char *FileName)
{
    MD_String8 Str = MD_S8ListJoin(GlobalMDArena, Stream, 0);
    OS_WriteEntireFile(FileName, S8Cast{.Data = Str.str, .Size = Str.size});
}

internal void 
WindowsBuild(str8 Source, str8 ExtraCompilerFlags, str8 ExtraLinkerFlags)
{
    arena *Arena = GlobalClingArena;
    str8 Output = PushS8(Arena, KB(2)); 
    
    SetSelectedArray(PushStr8Array(256));
    
    Str8ArrayAppend(S8("cmd /c \"C:\\msvc\\setup_x64.bat"));
    Str8ArrayAppend(S8("&&"));
    
    Str8ArrayAppend(S8("cl"));
    Str8ArrayAppend(S8("-MTd -Gm- -nologo -GR- -EHa- -Oi -FC -Z7"));
    Str8ArrayAppend(S8("-std:c++20 -Zc:strictStrings-"));
    Str8ArrayAppend(S8("-WX -W4 -wd4459 -wd4456 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4996 -wd4389 -wd4244 -wd5287"));
    Str8ArrayAppend(S8("-I" CLING_CODE_PATH));
    Str8ArrayAppend(ExtraCompilerFlags);
    
    Str8ArrayAppend(Source);
    
    Str8ArrayAppend(ExtraLinkerFlags);
    
    RunCommand();
}

internal void
LinuxMakeBuildCommand(str8 Source, 
                      str8 OutputName, 
                      b32 GCC, b32 Clang, b32 Asan, 
                      str8 ExtraFlags)
{
    Log(S8Fmt "\n", S8Arg(Cng_GetBaseFileName(Source)));
    
    SetSelectedArray(PushStr8Array(256));
    
    str8 CommonCompilerFlags = S8("-fsanitize-trap -nostdinc++ -D_GNU_SOURCE=1");
    str8 CommonWarningFlags = S8("-Wall -Wextra -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-sign-compare -Wno-double-promotion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-write-strings -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-parameter -Wno-unused-function");
    str8 LinkerFlags = S8("");
    
    str8 Compiler = {};
    if(0) {}
    else if(Clang) Compiler = S8("clang");
    else if(GCC) Compiler = S8("gcc");
    
    str8 DebugFlags = S8("-g -ggdb -g3");
    str8 ReleaseFlags = S8("-O3");
    
    str8 ClangCompilerFlags = S8("-fno-omit-frame-pointer -fdiagnostics-absolute-paths -ftime-trace -fsanitize-undefined-trap-on-error");
    str8 ClangWarningFlags = S8("-Wno-null-dereference -Wno-missing-braces -Wno-vla-cxx-extension -Wno-writable-strings -Wno-missing-designated-field-initializers -Wno-address-of-temporary -Wno-int-to-void-pointer-cast");
    str8 AsanFlags = S8("-fsanitize-trap -fsanitize=address");
    
    str8 GCCWarningFlags = S8("-Wno-cast-function-type -Wno-missing-field-initializers -Wno-int-to-pointer-cast");
    
    SetSelectedArray(PushStr8Array(256));
    Str8ArrayAppendMultiple(Compiler,
                            DebugFlags,
                            CommonCompilerFlags,
                            CommonWarningFlags);
    if(Clang)
    {
        Str8ArrayAppendMultiple(ClangCompilerFlags, ClangWarningFlags);
    }
    if(GCC)
    {
        Str8ArrayAppend(GCCWarningFlags);
    }
    if(Asan)
    {
        Str8ArrayAppend(AsanFlags);
    }
    
    Str8ArrayAppendMultiple(ExtraFlags,
                            S8("-o"), OutputName,
                            LinkerFlags,
                            Source);
    
}

ENTRY_POINT(EntryPoint)
{
    if(LaneIndex() == 0)
    {
        Cng_InitAndRebuildSelf(Params->ArgsCount, Params->Args, Params->Env);
        
        str8 CodePath = Cng_PathFromExe(Params->Args[0], S8(CLING_CODE_PATH));
        OS_ChangeDirectory((char *)CodePath.Data);
        
        b32 BuildEditor = true;
        
        b32 Windows = false;
        b32 Linux = false;
        
#if OS_WINDOWS
        Windows = true;
#elif OS_LINUX
        Linux = true;
#endif
        
        //- TODO(luca): 
        b32 Asan = false;
        b32 Clean = false;
        b32 Debug = true;
        b32 Release = false;
        b32 Clang = true;
        b32 GCC = false;
        // TODO(luca): should be inferred from `.base_build.h` existing.
        b32 Personal = true;
        b32 Slow = false;
        b32 Wine = false;
        //-
        
        if(BuildEditor)
        {
            Log("[editor build]\n");
            
            Log("Generating code...\n");
            {            
                GlobalMDArena = MD_ArenaAlloc();
                MD_String8 FileName = MD_S8Lit("./editor/tables.mdesk");
                MD_ParseResult Parse = MD_ParseWholeFile(GlobalMDArena, FileName);
                
                // Print metadesk errors
                for(MD_Message *Message = Parse.errors.first;
                    Message != 0;
                    Message = Message->next)
                {
                    MD_CodeLoc code_loc = MD_CodeLocFromNode(Message->node);
                    MD_PrintMessage(stdout, code_loc, Message->kind, Message->string);
                }
                if(Parse.errors.max_message_kind < MD_MessageKind_Error)
                {
                    MD_Node *Root = Parse.node->first_child;
                    
                    MD_String8List ShaderStream = {0};
                    MD_String8List CStream = {0};
                    
                    // Rcet shader attributes
                    {                
                        MD_Node *Table = MD_FirstNodeWithString(Root, MD_S8Lit("RectShaderAttributes"), 0);
                        
                        u32 Index = 0;
                        
                        MD_S8ListPushFmt(GlobalMDArena, &CStream,
                                         "s32 AttributesOffsets[] =\n{\n");
                        
                        for(MD_EachNode(Node, Table->first_child))
                        {
                            MD_Node *Name = MD_NodeAtIndex(Node->first_child, 0);
                            MD_Node *Size = MD_NodeAtIndex(Node->first_child, 1);
                            
                            MD_String8 TypeName = MD_S8Lit("null");
                            
                            if(0) {}
                            else if(MD_S8Match(Size->string, MD_S8Lit("1"), 0)) TypeName = MD_S8Lit("f32");
                            else if(MD_S8Match(Size->string, MD_S8Lit("4"), 0)) TypeName = MD_S8Lit("v4");
                            else InvalidPath();
                            
                            MD_S8ListPushFmt(GlobalMDArena, &ShaderStream, 
                                             "layout (location = %2d) in %3S %S;\n", 
                                             Index, TypeName, Name->string);
                            MD_S8ListPushFmt(GlobalMDArena, &CStream,
                                             "%S,\n", Size->string);
                            
                            Index += 1;
                        }
                        MD_S8ListPushFmt(GlobalMDArena, &CStream, "};\n");
                    }
                    
                    // UI Box flags
                    {
                        MD_Node *Table = MD_FirstNodeWithString(Root, MD_S8Lit("UI_BoxFlags"), 0);
                        
                        MD_S8ListPushFmt(GlobalMDArena, &CStream, "enum ui_box_flag\n{\n");
                        
                        u64 MaxWidth = 0;
                        for(MD_EachNode(Node, Table->first_child))
                        {
                            MaxWidth = Max(Node->string.size, MaxWidth);
                        }
                        
                        s32 Index = 0;
                        s32 Value = 0;
                        
                        for(MD_EachNode(Node, Table->first_child))
                        {
                            if(Index > 0) Value = 1;
                            MD_S8ListPushFmt(GlobalMDArena, &CStream, 
                                             "UI_BoxFlag_%-*S = (%d << %d),\n",
                                             MaxWidth, Node->string, Value, Index);
                            
                            Index += 1;
                        }
                        
                        MD_S8ListPushFmt(GlobalMDArena, &CStream, 
                                         "};\n"
                                         "typedef enum ui_box_flag ui_box_flag;\n");
                    }
                    
                    
                    MD_String8 Str = {0};
                    WriteStreamToFile(CStream, "./editor/generated/everything.c");
                    WriteStreamToFile(ShaderStream, "./editor/generated/rect_vert.glsl");
                }
            }
            
            OS_ChangeDirectory(BUILD);
            
            if(Linux)
            {
                str8 EditorFlags = S8("-DEDITOR_INTERNAL=1 -I" CLING_CODE_PATH);
                
                str8 LibsFileName = S8("editor_libs.o");
                str8 File = OS_ReadEntireFileIntoMemory((char *)LibsFileName.Data);
                if(!File.Size)
                {
                    LinuxMakeBuildCommand(S8("../source/editor/editor_libs.h"), 
                                          LibsFileName, 
                                          GCC, Clang, Asan, 
                                          StringCat(EditorFlags, S8(" -fPIC -x c++ -c "
                                                                    "-DEDITOR_SLOW_COMPILE=1")));
                    RunCommand();
                }
                
                LinuxMakeBuildCommand(S8("../source/editor/editor_app.cpp"), 
                                      S8("editor_app.so"),
                                      GCC, Clang, Asan,
                                      StringCat(EditorFlags, S8(" -fPIC --shared "
                                                                "-DEDITOR_SLOW_COMPILE=0")));
                Str8ArrayAppend(LibsFileName);
                RunCommand();
                
                str8 ExtraFlags = StringCat(EditorFlags, S8(" -lX11 -lGL -lGLX"));
                LinuxMakeBuildCommand(S8("../source/editor/editor_platform.cpp"),
                                      S8("editor"),
                                      GCC, Clang, Asan,
                                      ExtraFlags);
                RunCommand();
            }
            
            if(Windows)
            {
                str8 File = OS_ReadEntireFileIntoMemory("editor_libs.obj");
                if(!File.Size)
                {
                    WindowsBuild(S8("../source/editor/editor_libs.h"), 
                                 S8("-DEDITOR_INTERNAL=1 -DEDITOR_SLOW_COMPILE=1"
                                    "-Fmeditor_app.map -Foeditor_libs.obj"
                                    "-c /TP -LD "),
                                 S8("/link -opt:ref -incremental:no"));
                }
                {        
                    // Build app dll
                    WindowsBuild(S8("../source/editor/editor_app.cpp"),
                                 S8("-DEDITOR_INTERNAL=1 -DEDITOR_SLOW_COMPILE=0 -Fmeditor_app.map ./editor_libs.obj"), S8("-LD /link /DLL /EXPORT:UpdateAndRender"));
                    
                    // Build platform exe
                    WindowsBuild(S8("../source/editor/editor_platform.cpp"),
                                 S8("-DEDITOR_INTERNAL=1 -Feeditor.exe"),
                                 S8("/link -opt:ref -incremental:no user32.lib Gdi32.lib winmm.lib Opengl32.lib"));
                }
                
                if(0)
                {
                    Cng_RunCommand(S8("cmd /c pause"));
                }
            }
        }
    }
    
    return 0;
}
