#pragma once
#include "Reflect/Reflect.h"
#include "Engin5/Scene/Component.h"
#include "Engin5/Assets/Mesh.h"
#include "Generated/MeshRenderer_reflect_generated.h"

namespace Engin5
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

