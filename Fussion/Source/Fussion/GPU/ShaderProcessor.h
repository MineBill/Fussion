#pragma once
#include "Fussion/GPU/GPU.h"

namespace Fussion::GPU {
    class ShaderProcessor {
    public:
        /// Process shader file and resolve custom directives, such as '#import'.
        static auto ProcessFile(std::filesystem::path const& path) -> Maybe<std::string>;
    };
}
