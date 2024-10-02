#pragma once
#include "Fussion/Core/BitFlags.h"
#include "Fussion/Core/Types.h"
#include "Fussion/GPU/GPU.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector3.h"
#include "Fussion/Math/BoundingBox.h"

namespace Fussion {
    enum class DebugDrawFlag {
        DrawMeshNormals = 1 << 0,
        DrawMeshTangents = 1 << 1,
    };

    DECLARE_FLAGS(DebugDrawFlag, DebugDrawFlags);

    DECLARE_OPERATORS_FOR_FLAGS(DebugDrawFlags)

    struct DebugDrawContext {
        DebugDrawFlags flags {};
    };

    class Debug {
    public:
        static void initialize(
            GPU::Device& device,
            GPU::BindGroupLayout global_bind_group_layout,
            GPU::TextureFormat target_format);

        static void draw_box(BoundingBox const& box, Vector3 euler_angles, Vector3 size, f32 time = 0.0f, Color color = Color::Red);
        static void draw_box(BoundingBox const& box, f32 time = 0.0f, Color color = Color::Red);
        static void draw_line(Vector3 start, Vector3 end, f32 time = 0.0f, Color color = Color::Red);
        static void draw_cube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time = 0.0f, Color color = Color::Red);
        static void draw_cube(Vector3 min_extents, Vector3 max_extents, f32 time = 0.0f, Color color = Color::Red);
        static void draw_sphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time = 0.0f, Color color = Color::Red);

        static void render(GPU::RenderPassEncoder const& encoder);
        static void reset();
    };
}
