#if 0
#include "FussionPCH.h"
#include "Physics.h"

#include <fmt/printf.h>

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <cstdarg>
#include <cstdio>
#include <cstdarg>

namespace Fussion {
    class BroadPhaseLayerMapping;
    class ObjectVsBroadPhaseLayerFilterImpl;
    class ObjectLayerPairFilterImpl;
    class MyContactListener;
    class MyBodyActivationListener;

    struct PhysicsData {
        Ptr<JPH::TempAllocatorImpl> temp_allocator{};
        Ptr<JPH::JobSystemThreadPool> job_pool{};
        JPH::PhysicsSystem system;

        BroadPhaseLayerMapping* broad_phase_layer_mapping_impl;
        ObjectVsBroadPhaseLayerFilterImpl* object_vs_broad_phase_layer_filter_impl;
        MyContactListener* contact_listener;
        MyBodyActivationListener* body_activation_listener;
        ObjectLayerPairFilterImpl* object_layer_pair_filter_impl;
    };

    // PhysicsData physics;

    constexpr u32 MAX_BODIES = 1024;
    constexpr u32 NUM_BODY_MUTEXES = 0;
    constexpr u32 MAX_BODY_PAIRS = 1024;
    constexpr u32 MAX_CONTACT_CONSTRAINTS = 1024;

    class BroadPhaseLayerMapping : public JPH::BroadPhaseLayerInterface {
    public:
        BroadPhaseLayerMapping()
        {
            m_object_to_broad_phase_layer[Physics::ObjectLayers::NON_MOVING] = Physics::BroadPhaseLayers::NON_MOVING;
            m_object_to_broad_phase_layer[Physics::ObjectLayers::MOVING] = Physics::BroadPhaseLayers::MOVING;
        }

        virtual JPH::uint GetNumBroadPhaseLayers() const override
        {
            return Physics::BroadPhaseLayers::COUNT;
        }

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer in_layer) const override
        {
            return JPH::BroadPhaseLayer(m_object_to_broad_phase_layer[CAST(u16, in_layer)]);
        }

    private:
        u8 m_object_to_broad_phase_layer[Physics::ObjectLayers::COUNT];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer in_layer1, JPH::BroadPhaseLayer in_layer2) const override
        {
            switch (in_layer1) {
            case Physics::ObjectLayers::NON_MOVING:
                return in_layer2.GetValue() == Physics::BroadPhaseLayers::MOVING;
            case Physics::ObjectLayers::MOVING:
                return true;
            default:
                PANIC("Invalid object layer");
            }
        }
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer in_object1, JPH::ObjectLayer in_object2) const override
        {
            switch (in_object1) {
            case Physics::ObjectLayers::NON_MOVING:
                return in_object2 == Physics::ObjectLayers::MOVING; // Non-moving only collides with moving
            case Physics::ObjectLayers::MOVING:
                return true; // Moving collides with everything
            default:
                PANIC("Invalid object layer");
            }
        }
    };

    class MyContactListener final : public JPH::ContactListener {
    public:
        // See: ContactListener
        virtual JPH::ValidateResult OnContactValidate(JPH::Body const& in_body1, JPH::Body const& in_body2, JPH::RVec3Arg in_base_offset, JPH::CollideShapeResult const& in_collision_result) override
        {
            LOG_INFO("Contact validate");
            return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void OnContactAdded(JPH::Body const& in_body1, JPH::Body const& in_body2, JPH::ContactManifold const& in_manifold, JPH::ContactSettings& io_settings) override
        {
            LOG_INFO("Contact added");
        }

        virtual void OnContactPersisted(JPH::Body const& in_body1, JPH::Body const& in_body2, JPH::ContactManifold const& in_manifold, JPH::ContactSettings& io_settings) override
        {
            LOG_INFO("Contact persisted");
        }

        virtual void OnContactRemoved(JPH::SubShapeIDPair const& in_sub_shape_pair) override
        {
            LOG_INFO("Contact removed");
        }
    };

    class MyBodyActivationListener final : public JPH::BodyActivationListener {
    public:
        virtual void OnBodyActivated(JPH::BodyID const& in_body_id, u64 in_body_user_data) override
        {
            LOG_INFO("A body was activated");
        }

        virtual void OnBodyDeactivated(JPH::BodyID const& in_body_id, u64 in_body_user_data) override
        {
            LOG_INFO("A body was deactivated");
        }
    };

    static void jolt_trace(const char* fmt, ...)
    {
        std::array<char, 1024> formatted;

        va_list args;
        va_start(args, fmt);

        vsnprintf(formatted.data(), formatted.size(), fmt, args);

        va_end(args);

        LOG_INFOF("Jolt: {}", formatted.data());
    }

#ifdef JPH_ENABLE_ASSERTS
    static bool jolt_assert_failed(char const* in_expression, char const* in_message, char const* in_file, JPH::uint in_line)
    {
        LOG_ERRORF("Jolt assert failed: {}", in_message);
        LOG_ERRORF("\tExpression: {}", in_expression);
        LOG_ERRORF("\tFile: {}, Line: {}", in_file, in_line);
        return true;
    }
#endif

    void Physics::initialize()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = jolt_trace;
#ifdef JPH_ENABLE_ASSERTS
        JPH::AssertFailed = jolt_assert_failed;
#endif

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        physics.body_activation_listener = new MyBodyActivationListener;
        physics.broad_phase_layer_mapping_impl = new BroadPhaseLayerMapping;
        physics.contact_listener = new MyContactListener;
        physics.object_vs_broad_phase_layer_filter_impl = new ObjectVsBroadPhaseLayerFilterImpl;
        physics.object_layer_pair_filter_impl = new ObjectLayerPairFilterImpl;

        physics.system.Init(
            MAX_BODIES,
            NUM_BODY_MUTEXES,
            MAX_BODY_PAIRS,
            MAX_CONTACT_CONSTRAINTS,
            *physics.broad_phase_layer_mapping_impl,
            *physics.object_vs_broad_phase_layer_filter_impl,
            *physics.object_layer_pair_filter_impl);

        physics.system.SetBodyActivationListener(physics.body_activation_listener);
        physics.system.SetContactListener(physics.contact_listener);

        physics.temp_allocator = make_ptr<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        physics.job_pool = make_ptr<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
    }

    JPH::BodyInterface& Physics::body_interface()
    {
        return physics.system.GetBodyInterface();
    }
}
#endif
