#pragma once
#include <Fussion/Math/Color.h>
#include <Fussion/Core/BitFlags.h>
#include <Fussion/GPU/GPU.h>

#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    class PbrMaterial;
}

namespace Fussion {

    struct GPUSpotLight {};

    static_assert(sizeof(GPUSpotLight) == 1, "GPUSpotLight needs to be kept in sync with the shader equivalent");

    struct GPUPointLight {
        Vector3 position;
        Color color;
        f32 radius;
    };

    static_assert(sizeof(GPUPointLight) == 32, "GPUPointLight needs to be kept in sync with the shader equivalent");

    struct GPUDirectionalLight {
        struct ShaderStruct {
            std::array<Mat4, 4> light_space_matrix{};
            Vector4 direction{};
            Color color{};
        } shader_data;

        f32 split{};
    };

    static_assert(sizeof(GPUDirectionalLight) == 292, "GPUDirectionalLight needs to be kept in sync with the shader equivalent");
    static_assert(sizeof(GPUDirectionalLight::ShaderStruct) % 16 == 0, "GPUDirectionalLight needs to be kept in sync with the shader equivalent");

    enum class RenderState {
        None = 1 << 0,
        LightCollection = 1 << 1,
        Mesh = 1 << 2,
        Depth = 1 << 3,
        ObjectPicking = 1 << 4,
    };

    BITFLAGS(RenderState)

    enum class DrawPass: u32 {
        None = 1 << 0,
        Depth = 1 << 2,
        PBR = 1 << 3,

        All = Depth | PBR,
    };

    DECLARE_FLAGS(DrawPass, DrawPassFlags)

    DECLARE_OPERATORS_FOR_FLAGS(DrawPassFlags)

    struct PostProcessing {
        bool use_ssao{};
    };

    // NOTE: Ideally this would hold an actual material, that defines
    //       a shader to use. For now, we assume that all render objects
    //       are for the PBR pass.
    struct RenderObject {
        Vector3 position{};
        Mat4 world_matrix{};

        DrawPassFlags pass;
        PbrMaterial* material{};
        GPU::Buffer vertex_buffer{};
        GPU::Buffer index_buffer{};
        GPU::Buffer instance_buffer{};
        u32 index_count{};
    };

    using MeshBatchMap = std::unordered_map<GPU::HandleT, std::vector<size_t>>;

    struct RenderContext {
        RenderStateFlags render_flags;
        PostProcessing post_processing{};

        std::vector<GPUPointLight> point_lights{};
        std::vector<GPUDirectionalLight> directional_lights{};
        Mat4 current_light_space;

        std::vector<RenderObject> render_objects{};

        std::unordered_map<PbrMaterial*, MeshBatchMap> mesh_render_lists{};

        void add_render_object(RenderObject const& obj);
        void reset();
    };
}
