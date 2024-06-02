#pragma once
#include "Engin5/Core/Types.h"

/**
 * A simple clock/stopwatch class that uses native OS functions to measure elapsed time.
 */
class Clock
{
public:
    Clock();

    /**
     *
     * @return Elapsed time in milliseconds.
     */
    u64 Reset();

private:
    u64 m_TickCount{};
};
