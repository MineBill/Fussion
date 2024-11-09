#include "FussionPCH.h"
#include "Clock.h"
#include "Fussion/Core/Core.h"

#if defined(OS_WINDOWS)
#    include <Windows.h>
#elif defined(OS_LINUX)
#    include <time.h>
#endif

Clock::Clock()
{
#if defined(OS_WINDOWS)
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    m_clock_frequency = frequency.QuadPart;

    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);
    m_tick_count = integer.QuadPart;
#elif defined(OS_LINUX)
    (void)m_clock_frequency;
    timespec now {};
    clock_gettime(CLOCK_BOOTTIME, &now);

    m_tick_count = cast<u64>(now.tv_sec) * 1000000000LL + cast<u64>(now.tv_nsec);
#endif
}

f64 Clock::reset()
{
#if defined(OS_WINDOWS)
    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);

    auto diff = integer.QuadPart - m_tick_count;
    m_tick_count = integer.QuadPart;

    return CAST(f64, diff) / CAST(f64, m_clock_frequency);
#elif defined(OS_LINUX)
    timespec ts {};
    clock_gettime(CLOCK_BOOTTIME, &ts);

    u64 const now = cast<u64>(ts.tv_sec) * 1000000000LL + cast<u64>(ts.tv_nsec);
    u64 diff = (now - m_tick_count) / 1000; // to ms
    m_tick_count = now;

    return cast<f64>(diff) / 1000.0f / 1000.0f;
#endif
}
