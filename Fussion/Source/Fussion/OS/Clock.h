﻿#pragma once
#include "Fussion/Core/Types.h"

/**
 * A simple clock/stopwatch class that uses native OS functions to measure elapsed time.
 */
class Clock {
public:
    Clock();

    /**
     *
     * @return Elapsed time in milliseconds.
     */
    f64 Reset();

private:
    u64 m_ClockFrequency {};
    u64 m_TickCount {};
};
