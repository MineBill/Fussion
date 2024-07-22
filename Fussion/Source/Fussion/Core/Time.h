#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion {
class Application;

class Time {
    friend Application;

public:
    static f32 DeltaTime();

private:
    static void SetDeltaTime(f32 delta_time);
    static f32 m_DeltaTime;
};

}
