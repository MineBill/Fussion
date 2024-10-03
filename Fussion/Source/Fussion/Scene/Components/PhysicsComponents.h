#pragma once
#include <Fussion/Physics/Physics.h>
#include <Fussion/Scene/Component.h>

namespace Fussion {
    struct BoxCollider final : Component {
        Vector3 half_extent{};

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    struct SphereCollider final : Component {
        f32 radius{};

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    struct RigidBody final : Component {
        // BodyType type{};
        f32 linear_damping{};
        f32 angular_damping{};

        virtual void OnStart() override;
        virtual void OnUpdate(f32 delta) override;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
        u32 m_body_id{};
    };
}
