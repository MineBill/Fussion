#include "FussionPCH.h"
#include "Debug.h"

#include "Assets/AssetManager.h"
#include "Assets/AssetRef.h"
#include "Assets/ShaderAsset.h"
#include "Core/Time.h"
#include "OS/FileSystem.h"
#include "Fussion/Math/Color.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"

#include <ranges>
#include <glm/gtx/euler_angles.hpp>

namespace Fussion {

    struct Point {
        Vector3 Position{};
        f32 Thickness{};
        Color Color{};
    };

    struct DebugData {
        AssetRef<ShaderAsset> DebugShader{};
        GPU::Buffer VertexBuffer{};
        GPU::RenderPipeline Pipeline{};

        bool Initialized{ false };
        std::vector<Point> Points{};

        std::vector<Point> TimedPoints{};
        std::vector<f32> Timers{};

        GPU::Device Device{};
    };

    namespace {
        DebugData g_DebugData;
    }

    void Debug::initialize(GPU::Device& device, GPU::BindGroupLayout global_bind_group_layout, GPU::TextureFormat target_format)
    {
        if (g_DebugData.Initialized) {
            LOG_ERROR("Debug is already initialized");
            return;
        }
        g_DebugData.Device = device;

        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/DebugDraw.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "DebugDraw::Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main"sv,
            .fragment_entry_point = "fs_main"sv,
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        auto primitive = GPU::PrimitiveState::get_default();
        primitive.topology = GPU::PrimitiveTopology::LineList;

        std::array attributes{
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 0,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float,
                .shader_location = 1,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float4,
                .shader_location = 2,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

        GPU::RenderPipelineSpec rp_spec{
            .label = "DebugDraw::RenderPipeline"sv,
            .layout = layout,
            .vertex = {
                .attribute_layouts = { attribute_layout }
            },
            .primitive = primitive,
            .depth_stencil = GPU::DepthStencilState::get_default(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = target_format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        g_DebugData.Pipeline = device.create_render_pipeline(shader, rp_spec);

        GPU::BufferSpec spec{
            .label = "Debug::VertexBuffer"sv,
            .usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .size = 20'000 * sizeof(Point),
            .mapped = false,
        };

        g_DebugData.VertexBuffer = device.create_buffer(spec);
    }

    void Debug::draw_line(Vector3 start, Vector3 end, f32 time, Color color)
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

    void Debug::draw_cube(Vector3 center, Vector3 euler_angles, Vector3 size, f32 time, Color color)
    {
        auto half = size / 2;
        auto rotation = glm::mat3(glm::eulerAngleZXY(
            glm::radians(euler_angles.z),
            glm::radians(euler_angles.x),
            glm::radians(euler_angles.y)));

        draw_line(center + rotation * (Vector3{ -1, -1, 1 } * half), center + rotation * (Vector3{ 1, -1, 1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ -1, 1, 1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

        draw_line(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ 1, -1, -1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ -1, 1, -1 } * half), center + rotation * (Vector3{ 1, 1, -1 } * half), time, color);

        draw_line(center + rotation * (Vector3{ 1, -1, -1 } * half), center + rotation * (Vector3{ 1, -1, 1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ 1, 1, -1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

        draw_line(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ -1, -1, 1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ -1, 1, -1 } * half), center + rotation * (Vector3{ -1, 1, 1 } * half), time, color);

        draw_line(center + rotation * (Vector3{ -1, -1, -1 } * half), center + rotation * (Vector3{ -1, 1, -1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ 1, -1, -1 } * half), center + rotation * (Vector3{ 1, 1, -1 } * half), time, color);

        draw_line(center + rotation * (Vector3{ -1, -1, 1 } * half), center + rotation * (Vector3{ -1, 1, 1 } * half), time, color);
        draw_line(center + rotation * (Vector3{ 1, -1, 1 } * half), center + rotation * (Vector3{ 1, 1, 1 } * half), time, color);

    }

    void Debug::draw_cube(Vector3 min_extents, Vector3 max_extents, f32 time, Color color)
    {
        draw_cube((min_extents + max_extents) / 2.0f, {}, max_extents - min_extents, time, color);
    }

    void Debug::draw_sphere(Vector3 center, Vector3 euler_angles, f32 radius, f32 time, Color color)
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

            auto z0 = radius * Math::sin(lat0);
            auto z1 = radius * Math::sin(lat1);

            auto r0 = radius * Math::cos(lat0);
            auto r1 = radius * Math::cos(lat1);

            for (auto lon = 0; lon < latitude_segments; lon++) {
                auto lon0 = 2 * Math::PI * (CAST(f32, lon) / CAST(f32, longitude_segments));
                auto lon1 = 2 * Math::PI * (CAST(f32, lon + 1) / CAST(f32, longitude_segments));

                auto x0 = Math::cos(lon0) * r0;
                auto y0 = Math::sin(lon0) * r0;

                auto x1 = Math::cos(lon1) * r0;
                auto y1 = Math::sin(lon1) * r0;

                auto x2 = Math::cos(lon1) * r1;
                auto y2 = Math::sin(lon1) * r1;

                auto x3 = Math::cos(lon0) * r1;
                auto y3 = Math::sin(lon0) * r1;

                // Draw the quad formed by these four points
                draw_line(center + rotation * Vector3{ x0, y0, z0 }, center + rotation * Vector3{ x1, y1, z0 }, time, color);
                draw_line(center + rotation * Vector3{ x1, y1, z0 }, center + rotation * Vector3{ x2, y2, z1 }, time, color);
                draw_line(center + rotation * Vector3{ x2, y2, z1 }, center + rotation * Vector3{ x3, y3, z1 }, time, color);
                draw_line(center + rotation * Vector3{ x3, y3, z1 }, center + rotation * Vector3{ x0, y0, z0 }, time, color);
            }
        }
    }

    void Debug::render(GPU::RenderPassEncoder const& encoder)
    {
        auto line_count = g_DebugData.Points.size() + g_DebugData.TimedPoints.size();
        if (line_count == 0) {
            return;
        }
        VERIFY(line_count % 2 == 0);

        encoder.set_pipeline(g_DebugData.Pipeline);

        g_DebugData.Device.write_buffer(g_DebugData.VertexBuffer, 0, g_DebugData.Points.data(), g_DebugData.Points.size() * sizeof(Point));
        g_DebugData.Device.write_buffer(g_DebugData.VertexBuffer, g_DebugData.Points.size() * sizeof(Point), g_DebugData.TimedPoints.data(), g_DebugData.TimedPoints.size() * sizeof(Point));

        encoder.set_vertex_buffer(0, g_DebugData.VertexBuffer);
        encoder.draw({ 0, CAST(u32, line_count) }, { 0, 1 });
    }

    void Debug::reset()
    {
        g_DebugData.Points.clear();

        for (s32 i = CAST(s32, g_DebugData.TimedPoints.size()) - 1; i >= 0; i--) {
            auto& timer = g_DebugData.Timers[i];
            timer -= Time::delta_time();
            if (timer <= 0) {
                g_DebugData.TimedPoints.erase(g_DebugData.TimedPoints.begin() + i);
                g_DebugData.Timers.erase(g_DebugData.Timers.begin() + i);
            }
        }
    }
}
