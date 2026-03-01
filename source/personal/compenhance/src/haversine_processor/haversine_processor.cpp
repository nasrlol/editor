//~ Libraries
//- Standard 
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h> // for haversine
//- External  
#include "lr/lr.h"
#include "os.c"
#include "listing_0065_haversine_formula.cpp"
//- Project 
#include "haversine_processor.h"
#include "profiler.cpp"
#include "json.cpp"

//- Main 
int main(int ArgsCount, char *Args[])
{
    char *JSONFileName = 0;
    char *AnswersFileName = 0;
    u64 PairCount = 0;
    u64 Prof_Begin = 0;
    u64 Prof_End = 0;
    u64 OSFreq = GetOSTimerFreq();
    u64 OSStart = ReadOSTimer();
    u64 CPUStart = ReadCPUTimer();
    
    Prof_Markers = (prof_marker *)malloc(sizeof(prof_marker)*Gigabytes(1));
    
    Prof_Begin = ReadCPUTimer();
    
    // Parse command line
    {    
        if(ArgsCount >= 2)
        {
            JSONFileName = Args[1];
        }
        
        if(ArgsCount >= 3)
        {
            AnswersFileName = Args[2];
        }
    }
    
    if(JSONFileName)
    {
        str8 In = {};
        TimeScope("Read")
        {
            In = ReadEntireFileIntoMemory(JSONFileName);
        }
        
        if(In.Size)
        {    
            u64 Pos = 0;
            json_token *JSON = 0;
            
            TimeScope("Parse")
            {                
                JSON = JSON_ParseElement(In, &Pos);
            }
            
            prof_marker *Marker = Prof_Markers + Prof_MarkerCount;
            Prof_MarkerCount += 1;
            
            Marker->Label = "Some var init";
            Marker->Begin = ReadCPUTimer();
            
            json_token *PairsArray = 0;
            haversine_pair *HaversinePairs = 0;
            
            Marker->End = ReadCPUTimer();
            
            // Parse haversine pairs
            TimeScope("MiscSetup")
            {
                PairsArray = JSON_LookupIdentifierValue(JSON, S8Lit("\"pairs\""));
                
                // 30 bytes(characters) is the minimum for one pair
                u64 MaxPairCount = In.Size/30;
                HaversinePairs = (haversine_pair *)malloc(sizeof(haversine_pair)*MaxPairCount);
                
                for(json_token *Pair = PairsArray->Child;
                    Pair;
                    Pair = Pair->Next)
                {
                    haversine_pair *HaversinePair = HaversinePairs + PairCount;
                    HaversinePair->X0 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"x0\""))->Value);
                    HaversinePair->Y0 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"y0\""))->Value);
                    HaversinePair->X1 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"x1\""))->Value);
                    HaversinePair->Y1 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"y1\""))->Value);
                    
                    PairCount += 1;
                    
                    if(Pair->Next)
                    {
                        Assert(Pair->Next->Type == JSON_TokenType_Comma);
                        Pair = Pair->Next;
                    }
                }
            }
            JSON_FreeTokens(JSON);
            
            // Compute the sum
            TimeScope("Sum")
            {
                f64 AverageSum = 0;
                f64 AverageCoefficient = (1.0/(f64)PairCount);
                
                for(u64 PairIndex = 0;
                    PairIndex < PairCount;
                    PairIndex += 1)
                {
                    haversine_pair *HaversinePair = HaversinePairs + PairIndex;
                    
                    f64 EarthRadius = 6372.8;
                    f64 Sum = ReferenceHaversine(HaversinePair->X0, HaversinePair->Y0, 
                                                 HaversinePair->X1, HaversinePair->Y1,
                                                 EarthRadius);
                    AverageSum += AverageCoefficient*Sum;
                }
                
                TimeScope("MiscOutput")
                {                    
                    // TODO(luca): print average and difference between answer file 
                    
                    LogFormat("Average: %.16f\n", AverageSum);
                }
                
            }
            
            Prof_End = ReadCPUTimer();
            
            u64 TotalCPUElapsed = Prof_End - Prof_Begin;
            u64 TotalOSElapsed = ReadOSTimer() - OSStart;
            u64 CPUFreq = GuessCPUFreq(TotalOSElapsed, TotalCPUElapsed, OSFreq);
            
            LogFormat("\n"
                      "Elapsed: %llu\n"
                      "Total time: %.4fs (CPU freq %llu)\n", 
                      TotalCPUElapsed, (f64)TotalOSElapsed/(f64)OSFreq, CPUFreq);
            
            char *CurrentLabel = 0;
            for(EachIndex(MarkerIndex, Prof_MarkerCount))
            {
                prof_marker *Marker = Prof_Markers + MarkerIndex; 
                
                // NOTE(luca): Since labels are constant strings we can just check that the pointers match.
                
                u64 Begin = Marker->Begin;
                u64 End = Marker->End;
                u64 TotalElapsed = 0;
                u64 DuplicateCount = 0;
                while(MarkerIndex + 1 < Prof_MarkerCount && 
                      Marker->Label == Prof_Markers[MarkerIndex + 1].Label)
                {
                    TotalElapsed += Prof_Markers[MarkerIndex].End - Prof_Markers[MarkerIndex].Begin;
                    DuplicateCount += 1;
                    MarkerIndex += 1;
                }
                
                if(DuplicateCount)
                {
                    TotalElapsed += Prof_Markers[MarkerIndex].End - Prof_Markers[MarkerIndex].Begin;
                    Begin = 0;
                    End = TotalElapsed;
                }
                
                PrintTimeElapsed(Marker->Label, TotalCPUElapsed, Begin, End);
                
            }
            
            free(HaversinePairs);
            
        }
        else
        {
            LogFormat("ERROR: Cannot find file '%s' or its size is zero.\n", JSONFileName);
        }
    }
    else
    {
        LogFormat("ERROR: No file provided.\n");
        LogFormat("Usage: %s <data.json> [data.f64]\n", Args[0]); 
    }
    
    free(Prof_Markers);
    
    return 0;
}