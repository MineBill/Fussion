#include "FussionPCH.h"
#include "Clock.h"
#include "Fussion/Core/Core.h"

#if defined(OS_WINDOWS)
#include <Windows.h>
#elif defined(OS_LINUX)
#include <time.h>
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
    // m_TickCount = GetTickCount64();
#elif defined(OS_LINUX)
    timespec now{};
    clock_gettime(CLOCK_BOOTTIME, &now);

    m_TickCount = CAST(u64, now.tv_sec) * 1000000000LL + CAST(u64, now.tv_nsec);
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
    timespec ts{};
    clock_gettime(CLOCK_BOOTTIME, &ts);

    const u64 now = CAST(u64, ts.tv_sec) * 1000000000LL + CAST(u64, ts.tv_nsec);
    u64 diff = (now - m_TickCount) / 1000; // to ms
    m_TickCount = now;

    return CAST(f64, diff) / 1000.0f / 1000.0f;
#endif
}
