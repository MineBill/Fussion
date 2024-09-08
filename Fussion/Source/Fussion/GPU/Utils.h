#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion::GPU::Utils {
    struct RenderDoc {
        static void initialize();
        static void start_capture();
        static void end_capture();
    };
}
