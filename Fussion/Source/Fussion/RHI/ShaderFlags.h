#pragma once
#include "Fussion/Core/BitFlags.h"

namespace Fussion::RHI {
    enum class ShaderType {
        None = 1 << 0,
        Vertex = 1 << 1,
        Fragment = 1 << 2,
        Compute = 1 << 3,
    };

    DECLARE_FLAGS(ShaderType, ShaderTypeFlags)

    DECLARE_OPERATORS_FOR_FLAGS(ShaderTypeFlags)
}