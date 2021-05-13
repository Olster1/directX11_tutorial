/*
	We can't profile this file since we use it in the profiler to get the millseconds value for each function. 
	We might actually want to profile this one time, but not really important, as long as it just uses the QueryPerformanceCounter

*/
#include <cstdint>
#include <time.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;  

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#ifdef __APPLE__ 

#elif _WIN32
#include <timeapi.h>
static LARGE_INTEGER GlobalTimeFrequencyDatum;

inline s64 EasyTime_GetTimeCount()
{
    LARGE_INTEGER Time;
    QueryPerformanceCounter(&Time);
    
    return Time.QuadPart;
}

inline s64 EasyTime_GetSecondsCount() {
	return (EasyTime_GetTimeCount() / GlobalTimeFrequencyDatum.QuadPart);
} 

inline float EasyTime_GetSecondsElapsed(s64 CurrentCount, s64 LastCount)
{
    s64 Difference = CurrentCount - LastCount;
    float Seconds = (float)Difference / (float)GlobalTimeFrequencyDatum.QuadPart;
    
    return Seconds;
}

inline float EasyTime_GetMillisecondsElapsed(s64 CurrentCount, s64 LastCount)
{
    s64 Difference = CurrentCount - LastCount;
    assert(Difference >= 0); //user put them in the right order
    double Seconds = (float)Difference / (float)GlobalTimeFrequencyDatum.QuadPart;
	float millseconds = (float)(Seconds * 1000.0);     
    return millseconds;
    
}


inline void EasyTime_setupTimeDatums() {
	QueryPerformanceFrequency(&GlobalTimeFrequencyDatum);
	timeBeginPeriod(1);
}

static inline u64 EasyTime_getTimeStamp() {
    return (u64)time(NULL);
}
#endif