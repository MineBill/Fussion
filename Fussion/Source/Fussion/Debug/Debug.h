#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector3.h"

namespace Fussion {
class Debug {
public:
    static void Initialize();

    static void DrawLine(Vector3 start, Vector3 end);
};
}
