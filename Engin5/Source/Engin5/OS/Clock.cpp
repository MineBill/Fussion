#include "e5pch.h"
#include "Clock.h"
#include "Engin5/Core/Core.h"

#if defined(OS_WINDOWS)
#include <Windows.h>
#elif defined(OS_LINUX)
#endif

Clock::Clock()
{
#if defined(OS_WINDOWS)
    LARGE_INTEGER integer;
    QueryPerformanceCounter(&integer);
    m_TickCount = integer.QuadPart;
    // m_TickCount = GetTickCount64();
#elif defined(OS_LINUX)
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
#endif
}
