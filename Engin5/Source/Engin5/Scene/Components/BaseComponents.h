#pragma once
#include "Engin5/Core/Types.h"
#include "Engin5/Core/UUID.h"
#include "Reflect/Reflect.h"
#include "Engin5/Scene/Component.h"
#include "Generated/BaseComponents_reflect_generated.h"

namespace Engin5
{

    struct IdentityComponent
    {
        IdentityComponent(std::string const& name, const UUID id = UUID())
            : Name(name), Id(id)
        {
        }

        std::string Name;
        UUID Id{};
    };

    struct TransformComponent
    {
        TransformComponent() = default;

        Mat4 GetView() const
        {
            return Mat4(1.0);
        }

        Vector3 Position{};
        Vector3 EulerAngles{};
        Vector3 Scale{};
    };

    REFLECT_STRUCT(ShowInEditor)
    struct StupidComponent: public Component
    {
        REFLECT_GENERATED_BODY()
    public:
        COMPONENT(StupidComponent)

        REFLECT_PROPERTY(ShowInEditor)
        bool StupidValue = true;

        REFLECT_PROPERTY(ShowInEditor)
        bool StupidValuePepegas = true;

        REFLECT_PROPERTY(ShowInEditor)
        s64 Bit64Type = 2212;

        REFLECT_PROPERTY(ShowInEditor)
        s32 WhatIsThis = 22;

        REFLECT_PROPERTY(ShowInEditor)
        f32 WhatIsThisFloat = 22;

        REFLECT_PROPERTY()
        s32 StupidValue2 = 42;
    };
}