﻿#include "FussionPCH.h"
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
    m_ClockFrequency = frequency.QuadPart;

    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);
    m_TickCount = integer.QuadPart;
#elif defined(OS_LINUX)
    timespec now {};
    clock_gettime(CLOCK_BOOTTIME, &now);

    m_TickCount = CAST(u64, now.tv_sec) * 1000000000LL + CAST(u64, now.tv_nsec);
#endif
}

f64 Clock::Reset()
{
#if defined(OS_WINDOWS)
    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);

    auto diff = integer.QuadPart - m_TickCount;
    m_TickCount = integer.QuadPart;

    return CAST(f64, diff) / CAST(f64, m_ClockFrequency);
#elif defined(OS_LINUX)
    timespec ts {};
    clock_gettime(CLOCK_BOOTTIME, &ts);

    u64 const now = CAST(u64, ts.tv_sec) * 1000000000LL + CAST(u64, ts.tv_nsec);
    u64 diff = (now - m_TickCount) / 1000; // to ms
    m_TickCount = now;

    return CAST(f64, diff) / 1000.0f / 1000.0f;
#endif
}
