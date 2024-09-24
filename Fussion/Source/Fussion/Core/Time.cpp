#include "FussionPCH.h"
#include "Time.h"

namespace Fussion {
    f32 Time::m_delta_time{ 0.0f };
    f32 AverageDelta{ 0.0f };
    f32 SmoothDelta{ 0.0f };

    f32 Time::delta_time()
    {
        return m_delta_time;
    }

    f32 Time::smooth_delta_time()
    {
        return SmoothDelta;
    }

    void Time::set_delta_time(f32 delta_time)
    {
        m_delta_time = delta_time;

        constexpr auto frames = 16;
        static u64 counter = 0;

        AverageDelta += delta_time;
        if (counter++ % frames == 0) {
            SmoothDelta = AverageDelta / CAST(f32, frames);
            AverageDelta = 0.0f;
        }
    }
}
