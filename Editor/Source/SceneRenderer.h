#pragma once
#include "Fussion/Core/Types.h"

#include "Fussion/Scene/Scene.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Rendering/UniformBuffer.h"
#include "Fussion/Rendering/Pipelines/HDRPipeline.h"

namespace Fsn = Fussion;

// == Global Set == //

struct ViewData {
    Mat4 perspective{};
    Mat4 view{};
    Vector4 position{};

    // Mat4 RotationView{};
    // Vector2 ScreenSize{};
};

struct DebugOptions {
    b32 show_cascade_boxes;
    b32 show_cascade_colors;
};

struct GlobalData {
    f32 time{};
};

// == Scene Set == //

struct SceneData {
    Vector4 view_position{};
    Vector4 view_direction{};
    Color ambient_color{};
};

struct LightData {
    Fsn::GPUDirectionalLight::ShaderStruct directional_light{};

    Vector4 shadow_split_distances{};
};

// == == //

struct RenderCamera {
    Mat4 perspective, view;
    Vector3 position;
    f32 near, far;
    Vector3 direction;
};

struct RenderPacket {
    RenderCamera camera;
    Fsn::Scene* scene = nullptr;
    Vector2 size{};
};

constexpr s32 MAX_SHADOW_CASCADES = 4;
constexpr s32 SHADOWMAP_RESOLUTION = 4096;


class SceneRenderer {
public:
    struct RenderDebugOptions {
        s32 cascade_index{ 0 };
    } render_debug_options;

    Fussion::UniformBuffer<ViewData> scene_view_data;
    Fussion::UniformBuffer<LightData> scene_light_data;

    Fussion::UniformBuffer<DebugOptions> scene_debug_options;
    Fussion::UniformBuffer<GlobalData> scene_global_data;

    Fussion::UniformBuffer<SceneData> scene_scene_data;

    void init();
    void resize(Vector2 const& new_size);

    void render(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view = false);

    auto render_target() const -> Fussion::GPU::Texture const& { return m_scene_render_target; }

private:
    struct InstanceData {
        Mat4 model;
    };

    struct DepthInstanceData {
        Mat4 model;
        Mat4 light_space;
    };

    void setup_shadow_pass_render_target();
    void setup_shadow_pass();
    void depth_pass(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet);
    void pbr_pass(Fussion::GPU::CommandEncoder const& encoder, RenderPacket const& packet, bool game_view);

    void create_scene_render_target(Vector2 const& size);

    Fussion::HDRPipeline m_hdr_pipeline{};

    Fussion::GPU::Texture m_scene_render_target{};
    Fussion::GPU::Texture m_scene_render_depth_target{};

    Fussion::GPU::Texture m_shadow_pass_render_target{};
    std::array<Fussion::GPU::TextureView, MAX_SHADOW_CASCADES> m_shadow_pass_render_target_views{};

    Fussion::GPU::RenderPipeline m_simple_pipeline{}, m_grid_pipeline{}, m_pbr_pipeline{}, m_depth_pipeline{}, m_sky_pipeline{}, m_debug_pipeline{};

    Fussion::GPU::BindGroup m_global_bind_group{}, m_scene_bind_group{};
    Fussion::GPU::BindGroupLayout m_global_bind_group_layout{}, m_scene_bind_group_layout{}, m_object_bind_group_layout{}, m_object_depth_bgl{};

    std::vector<Fussion::GPU::Buffer> m_instance_buffer_pool{};

    Fussion::GPU::Buffer m_pbr_instance_buffer{}, m_depth_instance_buffer{};
    std::vector<u8> m_pbr_instance_staging_buffer{};
    std::vector<u8> m_depth_instance_staging_buffer{};

    Fussion::GPU::Sampler m_linear_sampler{};
    Fussion::GPU::Sampler m_shadow_sampler{};

    Vector2 m_render_area{};

    Fsn::RenderContext m_render_context{};

    std::vector<Fussion::GPU::BindGroup> m_object_groups_to_release{};
};
