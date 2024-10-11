#include "FussionPCH.h"
#include "Debug.h"

#include "Assets/AssetManager.h"
#include "Assets/AssetRef.h"
#include "Assets/ShaderAsset.h"
#include "Core/Time.h"
#include "Fussion/Math/Color.h"
#include "GPU/ShaderProcessor.h"
#include "OS/FileSystem.h"
#include "Rendering/Renderer.h"

#include <glm/gtx/euler_angles.hpp>
#include <ranges>

namespace Fussion {

    struct Point {
        Vector3 position {};
        f32 thickness {};
        Color color {};
    };

    struct DebugData {
        GPU::Buffer VertexBuffer {};
        AssetRef<ShaderAsset> Shader;

        bool Initialized { false };
        std::vector<Point> Points {};

        std::vector<Point> TimedPoints {};
        std::vector<f32> Timers {};

        GPU::Device Device {};
    };

    namespace {
        DebugData g_DebugData;
    }

    void Debug::Initialize(GPU::Device const& device, GPU::TextureFormat target_format)
    {
        if (g_DebugData.Initialized) {
            LOG_ERROR("Debug is already initialized");
            return;
        }
        g_DebugData.Device = device;

        constexpr auto path = "Assets/Shaders/Slang/Debug.slang";
        auto compiledShader = GPU::ShaderProcessor::CompileSlang(path).Unwrap();
        compiledShader.Metadata.UseBlending = true;
        compiledShader.Metadata.ParsedPragmas.push_back({ .Key = "topology", .Value = "lines" });

        auto shader = MakeRef<ShaderAsset>(compiledShader, std::vector { target_format });
        g_DebugData.Shader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);

        GPU::BufferSpec spec {
            .Label = "Debug::VertexBuffer"sv,
            .Usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .Size = 20'000 * sizeof(Point),
            .Mapped = false,
        };

        g_DebugData.VertexBuffer = device.CreateBuffer(spec);
    }

    void Debug::DrawBox(BoundingBox const& box, Vector3 euler_angles, Vector3 size, f32 time, Color color)
    {
        DrawCube(box.Center(), euler_angles, size, time, color);
    }

    void Debug::DrawBox(BoundingBox const& box, f32 time, Color color)
    {
        Vector3 v0 = Vector3(box.Min.x, box.Min.y, box.Min.z);
        Vector3 v1 = Vector3(box.Max.x, box.Min.y, box.Min.z);
        Vector3 v2 = Vector3(box.Max.x, box.Max.y, box.Min.z);
        Vector3 v3 = Vector3(box.Min.x, box.Max.y, box.Min.z);
        Vector3 v4 = Vector3(box.Min.x, box.Min.y, box.Max.z);
        Vector3 v5 = Vector3(box.Max.x, box.Min.y, box.Max.z);
        Vector3 v6 = Vector3(box.Max.x, box.Max.y, box.Max.z);
        Vector3 v7 = Vector3(box.Min.x, box.Max.y, box.Max.z);

        // Bottom face
        DrawLine(v0, v1, time, color);
        DrawLine(v1, v2, time, color);
        DrawLine(v2, v3, time, color);
        DrawLine(v3, v0, time, color);

        // Top face
        DrawLine(v4, v5, time, color);
        DrawLine(v5, v6, time, color);
        DrawLine(v6, v7, time, color);
        DrawLine(v7, v4, time, color);

        // Vertical edges
        DrawLine(v0, v4, time, color);
        DrawLine(v1, v5, time, color);
        DrawLine(v2, v6, time, color);
        DrawLine(v3, v7, time, color);
    }

    void Debug::DrawLine(Vector3 start, Vector3 end, f32 time, Color color)
    {
        if (time > 0) {
            g_DebugData.TimedPoints.emplace_back(start, 0, color);
            g_DebugData.TimedPoints.emplace_back(end, 0, color);

            g_DebugData.Timers.push_back(time);
            g_DebugData.Timers.push_back(time);
        } else [[likely]] {
            g_DebugData.Points.emplace_back(start, 0, color);
            g_DebugData.Points.emplace_back(end, 0, color);
        }
    }

    void Debug::DrawCube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time, Color color)
    {
        auto half = size / 2.0f;
        auto rotation = glm::mat3(glm::eulerAngleZXY(
            glm::radians(euler_angles.z),
            glm::radians(euler_angles.x),
            glm::radians(euler_angles.y)));

        DrawLine(center + rotation * (Vector3 { -1, -1, 1 } * half), center + rotation * (Vector3 { 1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { -1, 1, 1 } * half), center + rotation * (Vector3 { 1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3 { -1, -1, -1 } * half), center + rotation * (Vector3 { 1, -1, -1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { -1, 1, -1 } * half), center + rotation * (Vector3 { 1, 1, -1 } * half), time, color);

        DrawLine(center + rotation * (Vector3 { 1, -1, -1 } * half), center + rotation * (Vector3 { 1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { 1, 1, -1 } * half), center + rotation * (Vector3 { 1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3 { -1, -1, -1 } * half), center + rotation * (Vector3 { -1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { -1, 1, -1 } * half), center + rotation * (Vector3 { -1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3 { -1, -1, -1 } * half), center + rotation * (Vector3 { -1, 1, -1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { 1, -1, -1 } * half), center + rotation * (Vector3 { 1, 1, -1 } * half), time, color);

        DrawLine(center + rotation * (Vector3 { -1, -1, 1 } * half), center + rotation * (Vector3 { -1, 1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3 { 1, -1, 1 } * half), center + rotation * (Vector3 { 1, 1, 1 } * half), time, color);
    }

    void Debug::DrawCube(Vector3 min_extents, Vector3 max_extents, f32 time, Color color)
    {
        DrawCube((min_extents + max_extents) / 2.0f, {}, max_extents - min_extents, time, color);
    }

    void Debug::DrawSphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time, Color color)
    {
        constexpr auto segments = 10; // Number of segments per circle
        constexpr auto latitude_segments = segments;
        constexpr auto longitude_segments = segments;

        auto rotation = glm::mat3(glm::eulerAngleZXY(
            glm::radians(euler_angles.z),
            glm::radians(euler_angles.x),
            glm::radians(euler_angles.y)));

        // Draw latitude lines
        for (auto lat = 0; lat < latitude_segments; lat++) {
            auto lat0 = Math::PI * (-0.5 + (CAST(f32, lat) / CAST(f32, latitude_segments)));
            auto lat1 = Math::PI * (-0.5 + (CAST(f32, lat + 1) / CAST(f32, latitude_segments)));

            auto z0 = radius * Math::Sin(lat0);
            auto z1 = radius * Math::Sin(lat1);

            auto r0 = radius * Math::Cos(lat0);
            auto r1 = radius * Math::Cos(lat1);

            for (auto lon = 0; lon < latitude_segments; lon++) {
                auto lon0 = 2 * Math::PI * (CAST(f32, lon) / CAST(f32, longitude_segments));
                auto lon1 = 2 * Math::PI * (CAST(f32, lon + 1) / CAST(f32, longitude_segments));

                auto x0 = Math::Cos(lon0) * r0;
                auto y0 = Math::Sin(lon0) * r0;

                auto x1 = Math::Cos(lon1) * r0;
                auto y1 = Math::Sin(lon1) * r0;

                auto x2 = Math::Cos(lon1) * r1;
                auto y2 = Math::Sin(lon1) * r1;

                auto x3 = Math::Cos(lon0) * r1;
                auto y3 = Math::Sin(lon0) * r1;

                // Draw the quad formed by these four points
                DrawLine(center + rotation * Vector3 { x0, y0, z0 }, center + rotation * Vector3 { x1, y1, z0 }, time, color);
                DrawLine(center + rotation * Vector3 { x1, y1, z0 }, center + rotation * Vector3 { x2, y2, z1 }, time, color);
                DrawLine(center + rotation * Vector3 { x2, y2, z1 }, center + rotation * Vector3 { x3, y3, z1 }, time, color);
                DrawLine(center + rotation * Vector3 { x3, y3, z1 }, center + rotation * Vector3 { x0, y0, z0 }, time, color);
            }
        }
    }

    void Debug::Render(GPU::RenderPassEncoder const& encoder)
    {
        auto line_count = g_DebugData.Points.size() + g_DebugData.TimedPoints.size();
        if (line_count == 0) {
            return;
        }
        VERIFY(line_count % 2 == 0);

        auto shader = g_DebugData.Shader.Get();
        encoder.SetPipeline(shader->Pipeline());

        g_DebugData.Device.WriteBuffer(g_DebugData.VertexBuffer, 0, g_DebugData.Points.data(), g_DebugData.Points.size() * sizeof(Point));
        g_DebugData.Device.WriteBuffer(g_DebugData.VertexBuffer, g_DebugData.Points.size() * sizeof(Point), g_DebugData.TimedPoints.data(), g_DebugData.TimedPoints.size() * sizeof(Point));

        encoder.SetVertexBuffer(0, g_DebugData.VertexBuffer);
        encoder.Draw({ 0, CAST(u32, line_count) }, { 0, 1 });
    }

    void Debug::Reset()
    {
        g_DebugData.Points.clear();

        for (s32 i = CAST(s32, g_DebugData.TimedPoints.size()) - 1; i >= 0; i--) {
            auto& timer = g_DebugData.Timers[i];
            timer -= Time::DeltaTime();
            if (timer <= 0) {
                g_DebugData.TimedPoints.erase(g_DebugData.TimedPoints.begin() + i);
                g_DebugData.Timers.erase(g_DebugData.Timers.begin() + i);
            }
        }
    }
}
