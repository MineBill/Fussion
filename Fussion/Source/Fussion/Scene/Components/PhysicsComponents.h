#pragma once
#include <Fussion/Physics/Physics.h>
#include <Fussion/Scene/Component.h>

namespace Fussion {
    struct BoxCollider final : Component {
        Vector3 half_extent{};

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    struct SphereCollider final : Component {
        f32 radius{};

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    struct RigidBody final : Component {
        // BodyType type{};
        f32 linear_damping{};
        f32 angular_damping{};

        virtual void on_start() override;
        virtual void on_update(f32 delta) override;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        u32 m_body_id{};
    };
}
