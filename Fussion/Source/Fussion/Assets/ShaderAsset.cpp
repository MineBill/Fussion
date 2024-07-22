#include "e5pch.h"
#include "ShaderAsset.h"

namespace Fussion {
Ref<ShaderAsset> ShaderAsset::Create([[maybe_unused]] std::span<u8> data)
{
    return MakeRef<ShaderAsset>();
}
}
