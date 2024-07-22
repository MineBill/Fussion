#include "e5pch.h"
#include "PbrMaterial.h"

Fussion::PbrMaterial::PbrMaterial()
{
    MaterialUniformBuffer = RHI::UniformBuffer<MaterialBlock>::Create("Material");
}
