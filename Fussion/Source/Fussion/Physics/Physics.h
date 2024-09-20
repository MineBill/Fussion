#if 0

#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace Fussion {
    class PhysicsBody {
    public:
        u32 id() const { return m_id; }

    private:
        u32 m_id{};
    };

    enum class BodyType {
        Static,
        Dynamic,
        Kinematic,
    };

    class Physics {
    public:
        struct ObjectLayers {
            static constexpr u16 NON_MOVING = 0;
            static constexpr u16 MOVING = 1;
            static constexpr u16 COUNT = 2;
        };

        struct BroadPhaseLayers {
            static constexpr u8 NON_MOVING = 0;
            static constexpr u8 MOVING = 1;
            static constexpr u32 COUNT = 2;
        };

        static void initialize();

        static JPH::BodyInterface& body_interface();
    };
}
#endif