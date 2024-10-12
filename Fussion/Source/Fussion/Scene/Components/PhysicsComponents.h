#pragma once
#include <Fussion/Physics/Physics.h>
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class [[API]] BoxCollider final : public Component {
    public:
        COMPONENT_DEFAULT(BoxCollider)
        COMPONENT_DEFAULT_COPY(BoxCollider)
        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

        Vector3 half_extent{};
    };

    class [[API]] SphereCollider final : public Component {
    public:
        COMPONENT_DEFAULT(SphereCollider)
        COMPONENT_DEFAULT_COPY(SphereCollider)

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

        f32 radius{};
    };

    class [[API]] RigidBody final : public Component {
    public:
        COMPONENT_DEFAULT(RigidBody)
        COMPONENT_DEFAULT_COPY(RigidBody)

        virtual void OnStart() override;
        virtual void OnUpdate(f32 delta) override;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

        // BodyType type{};
        f32 linear_damping{};
        f32 angular_damping{};
    private:
        u32 m_body_id{};
    };
}
