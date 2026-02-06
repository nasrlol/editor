#include "lib/md5.h"
#include "base/base.h"
#include "editor/editor_platform.h"

#include "base/base.c"

#if OS_LINUX
# include "editor_platform_linux.cpp"
#elif OS_WINDOWS
# include "editor_platform_windows.cpp"
#endif

C_LINKAGE ENTRY_POINT(EntryPoint)
{
    if(LaneIdx() == 0)
    {
        arena *PermanentArena = ArenaAlloc(.Size = GB(3));
        arena *FrameArena = ArenaAlloc(.Size = GB(1));
        
        b32 *Running = PushStruct(PermanentArena, b32);
        *Running = true;
        
        app_offscreen_buffer Buffer = {};
        Buffer.Width = 1920/2;
        Buffer.Height = 1080/2;
        Buffer.BytesPerPixel = 4;
        Buffer.Pitch = Buffer.BytesPerPixel*Buffer.Width;
        Buffer.Pixels = PushArray(PermanentArena, u8, Buffer.Pitch*Buffer.Height);
        
        P_context PlatformContext = P_ContextInit(PermanentArena, &Buffer, Running);
        if(!PlatformContext)
        {
            ErrorLog("Could not initialize graphical context, running in headless mode.");
        }
        
        str8 ExeDirPath = {};
        {        
            u32 OnePastLastSlash = 0;
#if OS_LINUX || OS_ANDROID
            char *FileName = Params->Args[0];
            u32 SizeOfFileName = (u32)StringLength(FileName);
#elif OS_WINDOWS
            char *FileName = PushArray(PermanentArena, char, 1024);
            u32 SizeOfFileName = GetModuleFileNameA(0, FileName, 1024);
            Win32LogIfError();
#else
# error "OS not supported" 
#endif
            for EachIndex(Idx, SizeOfFileName)
            {
                if(FileName[Idx] == OS_SlashChar)
                {
                    OnePastLastSlash = (u32)Idx + 1;
                }
            }
            
            ExeDirPath.Data = (u8 *)FileName;
            ExeDirPath.Size = OnePastLastSlash;
        }
        
        app_memory AppMemory = {};
        AppMemory.ExeDirPath = ExeDirPath;
#if EDITOR_INTERNAL
        AppMemory.IsDebuggerAttached = GlobalDebuggerIsAttached;
#endif
        
        app_input _Input[2] = {};
        app_input *NewInput = &_Input[0];
        app_input *OldInput = &_Input[1];
        
        s64 LastCounter = OS_GetWallClock();
        s64 FlipWallClock = LastCounter;
#if EDITOR_FORCE_UPDATE_HZ
        f32 GameUpdateHz = EDITOR_FORCE_UPDATE_HZ;
#else
        f32 GameUpdateHz = 144.0f;
#endif
        f32 TargetSecondsPerFrame = 1.0f/GameUpdateHz; 
        
        app_code Code = ZeroStruct;
        
#if OS_LINUX || OS_ANDROID        
        str8 LibraryPath = S8("editor_app.so");
#else
        str8 LibraryPath = S8("editor_app.dll");
#endif
        Code.LibraryPath = PathFromExe(PermanentArena, AppMemory.ExeDirPath, LibraryPath);
        
        b32 Paused = false;
        
        OS_ProfileInit("P");
        while(*Running)
        {
            umm CPUBackPos = BeginScratch(FrameArena);
            
            OS_ProfileAndPrint("InitSetup");
            
            P_LoadAppCode(FrameArena, &Code, &AppMemory);
            OS_ProfileAndPrint("Code");
            
            // Input
            { 
                NewInput->Text.Count = 0;
                for EachElement(Idx, NewInput->MouseButtons)
                {
                    NewInput->MouseButtons[Idx].EndedDown = OldInput->MouseButtons[Idx].EndedDown;
                    NewInput->MouseButtons[Idx].HalfTransitionCount = 0;
                }
                for EachElement(Idx, NewInput->GameButtons)
                {
                    NewInput->GameButtons[Idx].EndedDown = OldInput->GameButtons[Idx].EndedDown;
                    NewInput->GameButtons[Idx].HalfTransitionCount = 0;
                }
                NewInput->dtForFrame = TargetSecondsPerFrame;
                
                P_ProcessMessages(PlatformContext, NewInput, &Buffer, Running);
            }
            
            OS_ProfileAndPrint("Messages");
            
            if(CharPressed(NewInput, 'p', PlatformKeyModifier_Alt)) Paused = !Paused;
            
            if(!Paused)
            {
                b32 ShouldQuit = Code.UpdateAndRender(ThreadContext, &AppMemory, PermanentArena, FrameArena, &Buffer, NewInput, OldInput);
                // NOTE(luca): Since UpdateAndRender can take some time, there could have been a signal sent to INT the app.
                ReadWriteBarrier;
                *Running = *Running && !ShouldQuit;
            }
            
            OS_ProfileAndPrint("UpdateAndRender");
            
            P_UpdateImage(PlatformContext, &Buffer);
            
            OS_ProfileAndPrint("UpdateImage");
            
            s64 WorkCounter = OS_GetWallClock();
            f32 WorkMSPerFrame = OS_MSElapsed(LastCounter, WorkCounter);
            // Sleep
            {            
                f32 SecondsElapsedForFrame = OS_SecondsElapsed(LastCounter, WorkCounter);
                if(SecondsElapsedForFrame < TargetSecondsPerFrame)
                {
                    f32 SleepUS = ((TargetSecondsPerFrame - 0.001f - SecondsElapsedForFrame)*1000000.0f);
                    if(SleepUS > 0)
                    {
                        // TODO(luca): Intrinsic
                        OS_Sleep((u32)SleepUS);
                    }
                    else
                    {
                        // TODO(luca): Logging
                    }
                    
                    f32 TestSecondsElapsedForFrame = OS_SecondsElapsed(LastCounter, OS_GetWallClock());
                    if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
                    {
                        // TODO(luca): Log missed sleep
                    }
                    
                    // NOTE(luca): This is to help against sleep granularity.
                    while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                    {
                        SecondsElapsedForFrame = OS_SecondsElapsed(LastCounter, OS_GetWallClock());
                    }
                }
                else
                {
                    // TODO(luca): Log missed frame rate!
                }
                
                LastCounter = OS_GetWallClock();
            }
            
            NewInput->Text.Buffer[NewInput->Text.Count].Codepoint = 0;
            
#if EDITOR_PROFILE
            // TODO(luca): Sometimes we hit more than 4ms/f
            if(WorkMSPerFrame >= 4000.0f) DebugBreakOnce;
#endif
            
            Swap(OldInput, NewInput);
            
            OS_ProfileAndPrint("Sleep");
            
            u8 Codepoint = (u8)NewInput->Text.Buffer[0].Codepoint;
            
#if 1            
            Log("'%c' (%d, %d) 1:%c 2:%c 3:%c", 
                ((Codepoint == 0) ?
                 '\a' : Codepoint),
                NewInput->MouseX, NewInput->MouseY,
                (NewInput->MouseButtons[PlatformMouseButton_Left  ].EndedDown ? 'x' : 'o'),
                (NewInput->MouseButtons[PlatformMouseButton_Middle].EndedDown ? 'x' : 'o'),
                (NewInput->MouseButtons[PlatformMouseButton_Right ].EndedDown ? 'x' : 'o')); 
            
            Log(" %.2fms/f", (f64)WorkMSPerFrame);
            Log("\n");
#endif
            
            FlipWallClock = OS_GetWallClock();
            
            EndScratch(FrameArena, CPUBackPos);
        }
    }
    
    return 0;
}
