#pragma once
#include "Fussion/Core/Types.h"

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
    f64 reset();

private:
    u64 m_clock_frequency{};
    u64 m_tick_count{};
};
