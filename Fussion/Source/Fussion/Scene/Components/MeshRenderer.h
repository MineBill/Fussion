#pragma once
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"

namespace Fussion
{
    class MeshRenderer: public Component
    {
    public:
        void OnUpdate(f32 delta) override;

        Ref<Mesh> Mesh;
    };
}

