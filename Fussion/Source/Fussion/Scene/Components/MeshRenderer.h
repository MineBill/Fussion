#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"

namespace Fussion
{
    class MeshRenderer: public Component
    {
        META_HPP_ENABLE_POLY_INFO(Component)
    public:
        MeshRenderer() = default;
        COMPONENT(MeshRenderer)

        void OnUpdate(f32 delta) override;

        AssetRef<Mesh> Mesh;
    };
}

