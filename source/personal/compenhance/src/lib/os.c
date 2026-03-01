#include <stdarg.h>
#include <stdio.h>

global_variable u8 LogBuffer[Kilobytes(64)];

#if OS_LINUX
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#endif

#if OS_WINDOWS
# include <windows.h>
# define RADDBG_MARKUP_IMPLEMENTATION
#else
# define RADDBG_MARKUP_STUBS
#endif
#include "raddbg_markup.h"
#include "raddbg_markup.h"

struct str8
{
    u8 *Data;
    u64 Size;
};
typedef struct str8 str8;
raddbg_type_view(str8, no_addr(array((char *)Data, Size)));
#define S8Lit(String) (str8){.Data = (u8 *)(String), .Size = (sizeof((String)) - 1)}

//- Debug utilities 
void AssertErrnoNotEquals(smm Result, smm ErrorValue)
{
    if(Result == ErrorValue)
    {
        int Errno = errno;
        Assert(0);
    }
}

void AssertErrnoEquals(smm Result, smm ErrorValue)
{
    if(Result != ErrorValue)
    {
        int Errno = errno;
        Assert(0);
    }
}

void LogFormat(char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    
#if OS_LINUX    
    int Length = stbsp_vsprintf((char *)LogBuffer, Format, Args);
    smm BytesWritten = write(STDOUT_FILENO, LogBuffer, Length);
    AssertErrnoEquals(BytesWritten, Length);
#elif OS_WINDOWS
    vprintf(Format, Args);
#endif
}

#if OS_LINUX
str8 ReadEntireFileIntoMemory(char *FileName)
{
    str8 Result = {};
    
    if(FileName)
    {
        int File = open(FileName, O_RDONLY);
        
        if(File != -1)
        {
            struct stat StatBuffer = {};
            int Error = fstat(File, &StatBuffer);
            AssertErrnoNotEquals(Error, -1);
            
            Result.Size = StatBuffer.st_size;
            Result.Data = (u8 *)mmap(0, Result.Size, PROT_READ, MAP_PRIVATE, File, 0);
            AssertErrnoNotEquals((smm)Result.Data, (smm)MAP_FAILED);
        }
    }
    
    return Result;
}

#elif OS_WINDOWS

str8 ReadEntireFileIntoMemory(char *FileName)
{
    str8 Result = {};
    
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = (u32)(FileSize.QuadPart);
            Result.Data = (u8 *)VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Data)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Data, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.Size = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    
                    if(Result.Data)
                    {
                        VirtualFree(Result.Data, 0, MEM_RELEASE);
                    }
                    
                    Result.Data = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    
    return Result;
}

#endif // OS_LINUX
