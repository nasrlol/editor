
//- Profiling 
#include "listing_0070_platform_metrics.cpp"
static void PrintTimeElapsed(char const *Label, u64 TotalTSCElapsed, u64 Begin, u64 End)
{
    u64 Elapsed = End - Begin;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s: %llu (%.2f%%)\n", Label, Elapsed, Percent);
}

u64 GuessCPUFreq(u64 OSElapsed, u64 CPUElapsed, u64 OSFreq)
{
    u64 CPUFreq = 0;
    if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
	
    return CPUFreq;
}

#define Concat_(A, B) A ## B
#define Concat(A, B) Concat_(A, B)
#define MacroVar(Name) Concat(Name, __LINE__)
#define MacroVarUnique(Name) Concat(Name, __COUNTER__)
#define DeferLoop(Start, End) \
for(int MacroVar(_i_) = (Start, 0); ! MacroVar(_i_); (MacroVar(_i_) += 1, End))

// From Jon Blow's MSVC finder.
template<typename F>
struct ScopeExit {
    ScopeExit(F f_) : f(f_) { }
    ~ScopeExit() { f(); }
    F f;
};

struct DeferHelper {
    template<typename F>
        ScopeExit<F> operator+(F f) { return f; }
};

#define Defer [[maybe_unused]] const auto & MacroVarUnique(DEFER_) = DeferHelper() + [&]()

struct prof_marker
{
    char *Label;
    u64 Begin;
    u64 End;
};
typedef struct prof_marker prof_marker;

global_variable u64 Prof_MarkerCount = 0;
global_variable prof_marker *Prof_Markers = 0;

#define MarkerVar MacroVar(Marker)

#define TimeScope(LabelName) \
prof_marker *MarkerVar = Prof_Markers + Prof_MarkerCount; \
Prof_MarkerCount += 1; \
MarkerVar->Label = (LabelName); \
DeferLoop(MarkerVar->Begin = ReadCPUTimer(), MarkerVar->End = ReadCPUTimer()) 

#define TimeFunction \
prof_marker *MarkerVar = Prof_Markers + Prof_MarkerCount; \
Prof_MarkerCount += 1; \
MarkerVar->Label = (char *)(__FUNCTION__); \
MarkerVar->Begin = ReadCPUTimer(); \
Defer { MarkerVar->End = ReadCPUTimer(); };

#define EachIndex(Index, Count) u64 Index = 0; Index < Count; Index += 1