#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector3.h"
#include "Fussion/RHI/RenderPass.h"
#include "Fussion/RHI/CommandBuffer.h"
#include "Fussion/Math/Color.h"

namespace Fussion {
    enum class DebugDrawFlag {
        DrawMeshNormals = 1 << 0,
        DrawMeshTangents = 1 << 1,
    };

    DECLARE_FLAGS(DebugDrawFlag, DebugDrawFlags);
    DECLARE_OPERATORS_FOR_FLAGS(DebugDrawFlags)

    struct DebugDrawContext {
        DebugDrawFlags Flags{};
    };

    class Debug {
    public:
        static void Initialize(Ref<RHI::RenderPass> const& render_pass);

        static void DrawLine(Vector3 start, Vector3 end, f32 time = 0.0f, Color color = Color::Red);
        static void DrawCube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time = 0.0f, Color color = Color::Red);
        static void DrawSphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time = 0.0f, Color color = Color::Red);

        static void Render(Ref<RHI::CommandBuffer> const& cmd, Ref<RHI::Resource> const& global_resource);
        static void Reset();
    };
}
