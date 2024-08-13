#include "e5pch.h"
#include "Time.h"

namespace Fussion {
    f32 Time::m_DeltaTime{ 0.0f };

    f32 Time::DeltaTime()
    {
        return m_DeltaTime;
    }

    void Time::SetDeltaTime(f32 delta_time)
    {
        m_DeltaTime = delta_time;
    }
}
