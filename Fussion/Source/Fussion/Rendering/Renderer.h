#pragma once
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/PbrMaterial.h>
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class Renderer {
    public:
        static void initialize(Window const& window);
        static void shutdown();

        static auto begin_rendering() -> Maybe<GPU::TextureView>;
        static void end_rendering(GPU::CommandBuffer cmd);
        static void resize(Vector2 const& new_size);

        [[nodiscard]]
        static auto device() -> GPU::Device&;

        [[nodiscard]]
        static auto surface() -> GPU::Surface&;

        [[nodiscard]]
        static auto gpu_instance() -> GPU::Instance&;

        [[nodiscard]]
        static auto default_material() -> AssetRef<PbrMaterial>;

        [[nodiscard]]
        static auto default_normal_map() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto white_texture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto black_texture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto white_cube_texture() -> GPU::Texture;

        static bool supports_pipeline_statistics();

        [[nodiscard]]
        static GPU::Texture generate_irradiance_map(GPU::Texture const& texture);

        static void create_default_resources();
    };
}
