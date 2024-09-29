#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Rendering/Pipelines/CubeMapSkybox.h"
#include "Fussion/Rendering/Pipelines/HDRPipeline.h"
#include "Fussion/Rendering/Pipelines/IBLIrradiance.h"
#include "Fussion/Rendering/Pipelines/SSAOBlur.h"
#include "Fussion/Rendering/UniformBuffer.h"
#include "Fussion/Scene/Scene.h"

namespace Fsn = Fussion;

// == Global Set == //

struct ViewData {
    Mat4 perspective {};
    Mat4 view {};
    Mat4 view_rotation_only {};
    Vector4 position {};
    Vector2 screen_size {}, _padding;
};

struct DebugOptions {
    b32 show_cascade_boxes;
    b32 show_cascade_colors;
};

struct GlobalData {
    f32 time {};
};

// == Scene Set == //

struct SceneData {
    Vector4 view_position {};
    Vector4 view_direction {};
    Color ambient_color {};
};

struct LightData {
    Fsn::GPUDirectionalLight::ShaderStruct directional_light {};

    Vector4 shadow_split_distances {};
};

// == == //

struct RenderCamera {
    Mat4 perspective, view;
    Mat4 rotation;
    Vector3 position;
    f32 near, far;
    Vector3 direction;
};

struct RenderPacket {
    RenderCamera camera;
    Fsn::Scene* scene = nullptr;
    Vector2 size {};
};

constexpr s32 MAX_SHADOW_CASCADES = 4;
constexpr s32 SHADOWMAP_RESOLUTION = 4096;

struct GBuffer {
    Fussion::GPU::Texture rt_position;
    Fussion::GPU::Texture rt_normal;
    Fussion::GPU::Texture rt_albedo;
    Fussion::GPU::RenderPipeline pipeline {};
    Fussion::GPU::BindGroupLayout bind_group_layout {};

    void init(Vector2 const& size, Fussion::GPU::BindGroupLayout const& global_bind_group_layout);
    void resize(Vector2 const& new_size);
    void do_pass(Fussion::GPU::CommandEncoder& encoder);
};

struct SSAO {
    Fussion::GPU::Texture render_target {};
    Fussion::GPU::Texture noise_texture {};
    Fussion::GPU::RenderPipeline pipeline {};
    Fussion::GPU::BindGroupLayout bind_group_layout {};
    Fussion::GPU::BindGroup bind_group {};
    Fussion::GPU::Sampler sampler {}, noise_sampler {};

    Fussion::GPU::Buffer samples_buffer {};

    void init(Vector2 const& size, GBuffer const& gbuffer, Fussion::GPU::BindGroupLayout const& global_bind_group_layout);
    void resize(Vector2 const& new_size, GBuffer const& gbuffer);
};

class SceneRenderer {
public:
    struct RenderDebugOptions {
        s32 cascade_index { 0 };
    } render_debug_options;

    struct Timings {
        f64 depth {};     // [0, 1]
        f64 gbuffer {};   // [2, 3]
        f64 ssao {};      // [4, 5]
        f64 ssao_blur {}; // [6, 7]
        f64 pbr {};       // [8, 9]
    } timings {};

    struct PipelineStatistics {
        struct Statistic {
            u64 vertex_shader_invocations {};
            u64 clipper_invocations {};
            // u64 clipper_primitives_out{};
            u64 fragment_shader_invocations {};
            // u64 compute_shader_invocations{};
        };

        Statistic gbuffer {};
        Statistic ssao {};
        Statistic pbr {};
    } pipeline_statistics {};

    Fussion::UniformBuffer<ViewData> scene_view_data;
    Fussion::UniformBuffer<LightData> scene_light_data;

    Fussion::UniformBuffer<DebugOptions> scene_debug_options;
    Fussion::UniformBuffer<GlobalData> scene_global_data;

    Fussion::UniformBuffer<SceneData> scene_scene_data;

    void init();
    void resize(Vector2 const& new_size);

    void render(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view = false);

    auto render_target() const -> Fussion::GPU::Texture const& { return m_scene_render_target; }

    GBuffer gbuffer;
    SSAO ssao;
    Fussion::SSAOBlur ssao_blur {};

private:
    struct InstanceData {
        Mat4 model;
    };

    struct DepthInstanceData {
        Mat4 model;
        Mat4 light_space;
    };

    void setup_scene_bind_group();
    void update_scene_bind_group(Fussion::GPU::Texture const& ssao_texture);

    void setup_shadow_pass_render_target();
    void setup_shadow_pass();
    void depth_pass(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet);
    void pbr_pass(Fussion::GPU::CommandEncoder const& encoder, RenderPacket const& packet, bool game_view);
    void setup_queries();

    void create_scene_render_target(Vector2 const& size);

    Fussion::HDRPipeline m_hdr_pipeline {};
    Fussion::CubeSkybox m_cube_skybox {};
    std::map<Fussion::AssetHandle, Fussion::GPU::Texture> m_environment_maps {};

    Fussion::GPU::Texture m_scene_render_target {};
    Fussion::GPU::Texture m_scene_render_depth_target {};

    Fussion::GPU::Texture m_shadow_pass_render_target {};
    std::array<Fussion::GPU::TextureView, MAX_SHADOW_CASCADES> m_shadow_pass_render_target_views {};

    Fussion::GPU::RenderPipeline m_simple_pipeline {}, m_grid_pipeline {}, m_pbr_pipeline {}, m_depth_pipeline {}, m_sky_pipeline {}, m_debug_pipeline {};

    Fussion::GPU::BindGroup m_global_bind_group {}, m_scene_bind_group {};
    Fussion::GPU::BindGroupLayout m_global_bind_group_layout {}, m_scene_bind_group_layout {}, m_object_bind_group_layout {}, m_object_depth_bgl {};

    std::vector<Fussion::GPU::Buffer> m_instance_buffer_pool {};

    Fussion::GPU::Buffer m_pbr_instance_buffer {}, m_depth_instance_buffer {};
    std::vector<u8> m_pbr_instance_staging_buffer {};
    std::vector<u8> m_depth_instance_staging_buffer {};

    Fussion::GPU::Sampler m_linear_sampler {};
    Fussion::GPU::Sampler m_shadow_sampler {};

    Fussion::GPU::QuerySet m_timings_set {};
    /// Used to resolve the query set into it.
    Fussion::GPU::Buffer m_timings_resolve_buffer {};
    /// Used to read from it on the CPU once we copy the resolve buffer into it.
    Fussion::GPU::Buffer m_timings_read_buffer {};

    Fussion::GPU::QuerySet m_statistics_query_set {};
    /// Used to resolve the query set into it.
    Fussion::GPU::Buffer m_statistics_resolve_buffer {};
    /// Used to read from it on the CPU once we copy the resolve buffer into it.
    Fussion::GPU::Buffer m_statistics_read_buffer {};

    Vector2 m_render_area {};

    Fsn::RenderContext m_render_context {};

    std::vector<Fussion::GPU::BindGroup> m_object_groups_to_release {};
};
