#include "e5pch.h"
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
    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);
    m_TickCount = integer.QuadPart;
    // m_TickCount = GetTickCount64();
#elif defined(OS_LINUX)
    timespec now{};
    clock_gettime(CLOCK_BOOTTIME, &now);

    m_TickCount = cast(u64, now.tv_sec) * 1000000000LL + cast(u64, now.tv_nsec);
#endif
}

u64 Clock::Reset()
{
#if defined(OS_WINDOWS)
    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    const auto diff = integer.QuadPart - m_TickCount;
    m_TickCount = integer.QuadPart;
    return cast(u64, diff * 1000000 / frequency.QuadPart);
#elif defined(OS_LINUX)
    timespec ts{};
    clock_gettime(CLOCK_BOOTTIME, &ts);

    const u64 now = cast(u64, ts.tv_sec) * 1000000000LL + cast(u64, ts.tv_nsec);
    u64 diff = (now - m_TickCount) / 1000; // to ms
    m_TickCount = now;

    return diff;
#endif
}
