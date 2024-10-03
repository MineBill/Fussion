#include "FussionPCH.h"
#include "Time.h"

namespace Fussion {
    f32 Time::m_DeltaTime { 0.0f };
    f32 AverageDelta { 0.0f };
    f32 SmoothDelta { 0.0f };

    f32 Time::DeltaTime()
    {
        return m_DeltaTime;
    }

    f32 Time::SmoothDeltaTime()
    {
        return SmoothDelta;
    }

    void Time::SetDeltaTime(f32 delta_time)
    {
        m_DeltaTime = delta_time;

        constexpr auto frames = 16;
        static u64 counter = 0;

        AverageDelta += delta_time;
        if (counter++ % frames == 0) {
            SmoothDelta = AverageDelta / CAST(f32, frames);
            AverageDelta = 0.0f;
        }
    }
}
