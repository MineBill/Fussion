#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion::GPU::Utils {
    struct RenderDoc {
        static void Initialize();
        static void StartCapture();
        static void EndCapture();
    };
}
