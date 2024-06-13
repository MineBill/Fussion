#pragma once
#include "Reflect/Reflect.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"
#include "Generated/MeshRenderer_reflect_generated.h"

namespace Fussion
{
    REFLECT_CLASS()
    class MeshRenderer: public Component
    {
        REFLECT_GENERATED_BODY()
    public:
        void OnUpdate(f32 delta) override;

        REFLECT_PROPERTY()
        Ref<Mesh> Mesh;
    };
}

