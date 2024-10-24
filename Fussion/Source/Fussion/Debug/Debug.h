﻿#pragma once
#include "Fussion/Core/BitFlags.h"
#include "Fussion/Core/Types.h"
#include "Fussion/GPU/GPU.h"
#include "Fussion/Math/BoundingBox.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector3.h"

namespace Fussion {
    enum class DebugDrawFlag {
        DrawMeshNormals = 1 << 0,
        DrawMeshTangents = 1 << 1,
    };

    DECLARE_FLAGS(DebugDrawFlag, DebugDrawFlags);

    DECLARE_OPERATORS_FOR_FLAGS(DebugDrawFlags)

    struct DebugDrawContext {
        DebugDrawFlags Flags {};
    };

    class Debug {
    public:
        static void Initialize(
            GPU::Device const& device,
            GPU::TextureFormat target_format);

        static void DrawBox(BoundingBox const& box, Vector3 euler_angles, Vector3 size, f32 time = 0.0f, Color color = Color::Red);
        static void DrawBox(BoundingBox const& box, f32 time = 0.0f, Color color = Color::Red);
        static void DrawLine(Vector3 start, Vector3 end, f32 time = 0.0f, Color color = Color::Red);
        static void DrawCube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time = 0.0f, Color color = Color::Red);
        static void DrawCube(Vector3 min_extents, Vector3 max_extents, f32 time = 0.0f, Color color = Color::Red);
        static void DrawSphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time = 0.0f, Color color = Color::Red);

        static void Render(GPU::RenderPassEncoder const& encoder);
        static void Reset();
    };
}
