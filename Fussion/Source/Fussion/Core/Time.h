#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion {
    class Application;

    class Time {
        friend Application;

    public:
        static f32 delta_time();
        static f32 smooth_delta_time();

    private:
        static void set_delta_time(f32 delta_time);
        static f32 m_delta_time;
    };
}
