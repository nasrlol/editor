//~ Libraries
#include "lr/lr.h"
PUSH_WARNINGS
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include <math.h>
#include "listing_0065_haversine_formula.cpp"
POP_WARNINGS

//~ Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#if OS_LINUX
# include <fcntl.h>
# include <unistd.h>
# include <sys/mman.h>
#endif

#if OS_WINDOWS
# include <Windows.h>
# define _CRT_SECURE_NO_WARNINGS 1
# define WIN32_LEAN_AND_MEAN 1
#endif

#include "haversine_random.h"

//~ Macro's
#define MemoryCopy memcpy

//~ Types
#include "generated/types.h"

//~ Helpers

void WriteMemoryTofile(umm Size, void *Memory, umm PairCount, char *Name, char *Extension)
{
    char FileName[256] = {};
    stbsp_sprintf(FileName, "data_%llu_%s.%s", PairCount, Name, Extension);
    
#if OS_LINUX
    int File = open(FileName, O_RDWR|O_CREAT|O_TRUNC, 0600);
    if(File != -1)
    {
        smm BytesWritten = write(File, Memory, Size);
        Assert(BytesWritten == Size);
    }
    else
    {
        // TODO(luca): Logging
    }
    
#elif OS_WINDOWS
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, (DWORD)Size, &BytesWritten, 0))
        {
            // NOTE(casey): File written successfully
            Assert(BytesWritten == Size);
        }
        else
        {
            // TODO(casey): Logging
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }
#endif
}

internal void *
PlatformGetMemory(umm Size)
{
    void *Result = 0;
    
#if OS_LINUX
    Result = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
#elif OS_WINDOWS
    Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#endif
    
    return Result;
}


//~ Main
int main(int ArgsCount, char *Args[], char *Env[])
{
    // 1. haversine_generator [uniform/cluster] [random seed] [number of pairs to generate]
    
    if(ArgsCount >= 4)
    {
        u32 Method = 0;
        u64 RandomSeed = 0;;
        u64 PairCount = 0;
        
        char *MethodName = Args[1];
        char *SeedString = Args[2];
        
        u64 ClusterCountLeft = U64Max;
        
        f64 MaxAllowedX = 180.0;
        f64 MaxAllowedY = 90.0;
        f64 XCenter = 0.0;
        f64 YCenter = 0.0;
        f64 XRadius = MaxAllowedX;
        f64 YRadius = MaxAllowedY;
        
        if(!strcmp(MethodName, "cluster"))
        {
            ClusterCountLeft = 0;
        }
        else if(strcmp(MethodName, "uniform"))
        {
            MethodName = "uniform";
            printf("Warning: Unknown method '%s', uniform used.\n", MethodName);
        }
        
        RandomSeed = atoll(Args[2]);
        PairCount = atoll(Args[3]);
        
        u64 MaxPairCount = (1LL << 34);
        if(PairCount < MaxPairCount)
        {
            umm JsonMemorySize = Gigabytes(4);
            u8 *JsonMemory = (u8 *)PlatformGetMemory(JsonMemorySize);
            u8 *JsonOut = JsonMemory;
            
            umm BinMemorySize = Gigabytes(4);
            u8 *BinMemory = (u8 *)PlatformGetMemory(BinMemorySize);;
            u8 *BinOut = BinMemory;
            
            JsonOut += stbsp_sprintf((char *)JsonOut, 
                                     "{\n"
                                     " \"pairs\":\n"
                                     "  [\n");
            
            random_series Series = {};
            Seed(&Series, RandomSeed);
            
            u64 ClusterCountMax = 1 + (PairCount / 64);
            
            f64 SumCoefficient = 1.0/(f64)PairCount;
            f64 Sum = 0;
            
            for(u32 PairIndex = 0;
                PairIndex < PairCount;
                PairIndex += 1)
            {
                if(ClusterCountLeft-- == 0)
                {
                    ClusterCountLeft = ClusterCountMax;
                    
                    XCenter = RandomBetween(&Series, -MaxAllowedX, MaxAllowedX);
                    YCenter = RandomBetween(&Series, -MaxAllowedY, MaxAllowedY);
                    XRadius = RandomBetween(&Series, 0, MaxAllowedX);
                    YRadius = RandomBetween(&Series, 0, MaxAllowedY);
                }
                
                f64 X0 = RandomDegree(&Series, XCenter, XRadius, MaxAllowedX);
                f64 Y0 = RandomDegree(&Series, YCenter, YRadius, MaxAllowedY);
                f64 X1 = RandomDegree(&Series, XCenter, XRadius, MaxAllowedX);
                f64 Y1 = RandomDegree(&Series, YCenter, YRadius, MaxAllowedY);
                
                f64 EarthRadius = 6372.8;
                f64 Distance = ReferenceHaversine(X0, Y0, X1, Y1, EarthRadius);
                Sum += SumCoefficient*Distance;
                
                *(f64 *)BinOut = Distance;
                BinOut += sizeof(Distance);
                
                const char *Separator = (PairIndex == PairCount - 1) ? "\n" : ",\n";
                JsonOut += stbsp_sprintf((char *)JsonOut, 
                                         "   { \"x0\": %.16f, \"y0\": %.16f, \"x1\": %.16f, \"y1\": %.16f }%s", 
                                         X0, Y0, X1, Y1, Separator);
            }
            
            *(f64 *)BinOut = Sum;
            BinOut += sizeof(Sum);
            
            printf("Method: %s\n"
                   "Random seed: %llu\n"
                   "Pairs count: %llu\n"
                   , MethodName, RandomSeed, PairCount);
            printf("Average sum: %.16f\n", Sum);
            
            JsonOut += stbsp_sprintf((char *)JsonOut, 
                                     "  ]\n"
                                     "}\n");
            WriteMemoryTofile(JsonOut - JsonMemory, JsonMemory, PairCount, "flex", "json");
            WriteMemoryTofile(BinOut - BinMemory, BinMemory, PairCount, "haveranswer", "f64");
        }
        else
        {
            printf("Massive files unsupported. Number of pairs must be less than %llu.\n", MaxPairCount);
        }
    }
    else
    {
        printf("Usage: %s [uniform/cluster] [random seed] [number of pairs to generate]\n", 
               Args[0]);
    }
    
    return 0;
}
