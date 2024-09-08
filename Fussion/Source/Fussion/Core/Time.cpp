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
        static f32 past_deltas[frames]{ 0.0f };
        static u64 counter = 0;

        // past_deltas[0] = delta_time;
        // for (int i = frames - 1; i >= 1; --i) {
        //     past_deltas[i] = past_deltas[i - 1];
        // }
        //
        // f32 avg{ 0.0f };
        // for (size_t i = 0; i < frames; ++i)
        //     avg += past_deltas[i];
        // SmoothDelta = avg / CAST(f32, frames);

        AverageDelta += delta_time;
        if (counter++ % frames == 0) {
            SmoothDelta = AverageDelta / CAST(f32, frames);
            AverageDelta = 0.0f;
        }
    }
}
