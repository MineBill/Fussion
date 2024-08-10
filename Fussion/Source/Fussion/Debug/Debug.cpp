#include "e5pch.h"
#include "Debug.h"

#include "Assets/AssetManager.h"
#include "Assets/AssetRef.h"
#include "Assets/ShaderAsset.h"
#include "Core/Time.h"
#include "Fussion/RHI/Shader.h"
#include "OS/FileSystem.h"
#include "RHI/Device.h"
#include "RHI/ShaderCompiler.h"
#include "Fussion/Math/Color.h"

#include <ranges>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Fussion {

    struct Point {
        Vector3 Position{};
        f32 Thickness{};
        Color Color{};
    };

    struct DebugData {
        AssetRef<ShaderAsset> DebugShader{};
        Ref<RHI::Buffer> VertexBuffer{};

        bool Initialized{ false };
        std::vector<Point> Points{};

        std::vector<Point> TimedPoints{};
        std::vector<f32> Timers{};
    };

    static DebugData g_DebugData;

    void Debug::Initialize(Ref<RHI::RenderPass> const& render_pass)
    {
        if (g_DebugData.Initialized) {
            LOG_ERROR("Debug is already initialized");
            return;
        }
        auto src = FileSystem::ReadEntireFile("Assets/Shaders/Debug.shader");
        VERIFY(src.has_value());
        auto result = RHI::ShaderCompiler::Compile(*src, "Debug.shader");
        VERIFY(result.HasValue());

        auto shader = ShaderAsset::Create(render_pass, result->ShaderStages, result->Metadata);
        g_DebugData.DebugShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, "Assets/Shaders/Debug.shader");

        g_DebugData.Initialized = true;

        auto spec = RHI::BufferSpecification{
            .Label = "Debug Vertex Buffer",
            .Usage = RHI::BufferUsage::Vertex,
            .Size = 20'000 * sizeof(Point),
            .Mapped = true
        };
        g_DebugData.VertexBuffer = RHI::Device::Instance()->CreateBuffer(spec);
    }

    void Debug::DrawLine(Vector3 start, Vector3 end, f32 time, Color color)
    {
        if (time > 0) {
            g_DebugData.TimedPoints.emplace_back(start, 0, color);
            g_DebugData.TimedPoints.emplace_back(end, 0, color);

            g_DebugData.Timers.push_back(time);
            g_DebugData.Timers.push_back(time);
        } else {
            g_DebugData.Points.emplace_back(start, 0, color);
            g_DebugData.Points.emplace_back(end, 0, color);
        }
    }

    void Debug::DrawCube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time, Color color)
    {
        auto half = size / 2;
        auto rotation = glm::mat3(glm::eulerAngleZXY(
            glm::radians(euler_angles.Z),
            glm::radians(euler_angles.X),
            glm::radians(euler_angles.Y)));

        DrawLine(center + rotation * (Vector3{ -1, -1, 1 } * half), center + rotation * (Vector3{ 1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ -1, 1, 1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ 1, -1, -1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ -1, 1, -1 } * half), center + rotation * (Vector3{ 1, 1, -1 } * half), time, color);

        DrawLine(center + rotation * (Vector3{ 1, -1, -1 } * half), center + rotation * (Vector3{ 1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ 1, 1, -1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ -1, -1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ -1, 1, -1 } * half), center + rotation * (Vector3{ -1, 1, 1 } * half), time, color);

        DrawLine(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ -1, 1, -1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ 1, -1, -1 } * half), center + rotation * (Vector3{ 1, 1, -1 } * half), time, color);

        DrawLine(center + rotation * (Vector3{ -1, -1, 1 } * half), center + rotation * (Vector3{ -1, 1, 1 } * half), time, color);
        DrawLine(center + rotation * (Vector3{ 1, -1, 1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

    }

    void Debug::DrawSphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time, Color color)
    {
        constexpr auto segments = 10; // Number of segments per circle
        constexpr auto latitude_segments = segments;
        constexpr auto longitude_segments = segments;

        auto rotation = glm::mat3(glm::eulerAngleZXY(
            glm::radians(euler_angles.Z),
            glm::radians(euler_angles.X),
            glm::radians(euler_angles.Y)));

        // Draw latitude lines
        for (auto lat = 0; lat < latitude_segments; lat++) {
            auto lat0 = Math::Pi * (-0.5 + (CAST(f32, lat) / CAST(f32, latitude_segments)));
            auto lat1 = Math::Pi * (-0.5 + (CAST(f32, lat + 1) / CAST(f32, latitude_segments)));

            auto z0 = radius * Math::Sin(lat0);
            auto z1 = radius * Math::Sin(lat1);

            auto r0 = radius * Math::Cos(lat0);
            auto r1 = radius * Math::Cos(lat1);

            for (auto lon = 0; lon < latitude_segments; lon++) {
                auto lon0 = 2 * Math::Pi * (CAST(f32, lon) / CAST(f32, longitude_segments));
                auto lon1 = 2 * Math::Pi * (CAST(f32, lon + 1) / CAST(f32, longitude_segments));

                auto x0 = Math::Cos(lon0) * r0;
                auto y0 = Math::Sin(lon0) * r0;

                auto x1 = Math::Cos(lon1) * r0;
                auto y1 = Math::Sin(lon1) * r0;

                auto x2 = Math::Cos(lon1) * r1;
                auto y2 = Math::Sin(lon1) * r1;

                auto x3 = Math::Cos(lon0) * r1;
                auto y3 = Math::Sin(lon0) * r1;

                // Draw the quad formed by these four points
                DrawLine(center + rotation * Vector3{ x0, y0, z0 }, center + rotation * Vector3{ x1, y1, z0 }, time, color);
                DrawLine(center + rotation * Vector3{ x1, y1, z0 }, center + rotation * Vector3{ x2, y2, z1 }, time, color);
                DrawLine(center + rotation * Vector3{ x2, y2, z1 }, center + rotation * Vector3{ x3, y3, z1 }, time, color);
                DrawLine(center + rotation * Vector3{ x3, y3, z1 }, center + rotation * Vector3{ x0, y0, z0 }, time, color);
            }
        }
    }

    void Debug::Render(Ref<RHI::CommandBuffer> const& cmd, Ref<RHI::Resource> global_resource)
    {
        auto line_count = g_DebugData.Points.size() + g_DebugData.TimedPoints.size();
        if (line_count == 0) {
            return;
        }
        VERIFY(line_count % 2 == 0);

        auto shader = g_DebugData.DebugShader.Get()->GetShader();
        cmd->UseShader(shader);

        g_DebugData.VertexBuffer->SetData(g_DebugData.Points.data(), g_DebugData.Points.size() * sizeof(Point));
        g_DebugData.VertexBuffer->SetData(g_DebugData.TimedPoints.data(), g_DebugData.TimedPoints.size() * sizeof(Point), g_DebugData.Points.size() * sizeof(Point));

        cmd->BindResource(global_resource, shader, 0);
        cmd->BindBuffer(g_DebugData.VertexBuffer);
        cmd->Draw(line_count, 1);
    }

    void Debug::Reset()
    {
        g_DebugData.Points.clear();

        for (s32 i = g_DebugData.TimedPoints.size() - 1; i >= 0; i--) {
            auto& timer = g_DebugData.Timers[i];
            timer -= Time::DeltaTime();
            if (timer <= 0) {
                g_DebugData.TimedPoints.erase(g_DebugData.TimedPoints.begin() + i);
                g_DebugData.Timers.erase(g_DebugData.Timers.begin() + i);
            }
        }
    }
}
