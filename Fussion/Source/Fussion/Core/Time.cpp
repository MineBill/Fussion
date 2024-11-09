#include "FussionPCH.h"
#include "Time.h"

namespace Fussion {
    f32 Time::m_delta_time { 0.0f };
    f32 g_average_delta { 0.0f };
    f32 g_smooth_delta { 0.0f };

    f32 Time::delta_time()
    {
        return m_delta_time;
    }

    f32 Time::smooth_delta_time()
    {
        return g_smooth_delta;
    }

    void Time::set_delta_time(f32 delta_time)
    {
        m_delta_time = delta_time;

        constexpr auto frames = 16;
        static u64 counter = 0;

        g_average_delta += delta_time;
        if (counter++ % frames == 0) {
            g_smooth_delta = g_average_delta / CAST(f32, frames);
            g_average_delta = 0.0f;
        }
    }
}
