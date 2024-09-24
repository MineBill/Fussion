#include "FussionPCH.h"
#include "PhysicsComponents.h"

#include "Scene/Entity.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Fussion {
    JPH::RVec3 v(Vector3 const& v)
    {
        return JPH::RVec3(v.x, v.y, v.z);
    }

    void RigidBody::on_start()
    {
        JPH::Ref<JPH::Shape> shape;
        if (m_owner->has_component<SphereCollider>()) {
            auto sphere = m_owner->get_component<SphereCollider>();

            JPH::SphereShapeSettings settings(sphere->radius);
            auto result = settings.Create();
            VERIFY(result.IsValid());

            shape = result.Get();
        } else if (m_owner->has_component<BoxCollider>()) {
            auto box = m_owner->get_component<BoxCollider>();

            JPH::BoxShapeSettings settings(v(box->half_extent));
            auto result = settings.Create();
            VERIFY(result.IsValid());

            shape = result.Get();
        }

        // auto& interface = Physics::body_interface();
        //
        // auto quat = glm::toQuat(glm::orientate3(glm::vec3(m_owner->transform.euler_angles)));
        // JPH::BodyCreationSettings settings(
        //     shape.GetPtr(),
        //     v(m_owner->transform.position),
        //     JPH::QuatArg(quat.x, quat.y, quat.z, quat.w),
        //     JPH::EMotionType::Dynamic,
        //     Physics::ObjectLayers::MOVING);
        //
        // auto* body = interface.CreateBody(settings);
        // body->SetUserData(m_owner->handle());
        // m_body_id = body->GetID().GetIndex();
        //
        // interface.AddBody(body->GetID(), JPH::EActivation::Activate);
        //
        // auto* motion = body->GetMotionProperties();
        // if (motion != nullptr) {
        //     motion->SetLinearDamping(linear_damping);
        //     motion->SetAngularDamping(angular_damping);
        // }
    }

    void RigidBody::on_update(f32 delta)
    {
        (void)delta;
    }

    void RigidBody::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
    }

    void RigidBody::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
    }
}
