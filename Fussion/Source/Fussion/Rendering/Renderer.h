#pragma once
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/PbrMaterial.h>
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class Renderer {
    public:
        static void initialize(Window const& window);
        static void shutdown();

        static Renderer& inst() { return *s_renderer; }

        static auto begin_rendering() -> Maybe<GPU::TextureView>;
        static void end_rendering(GPU::CommandBuffer cmd);
        static void resize(Vector2 const& new_size);

        static auto device() -> GPU::Device&
        {
            return s_renderer->m_device;
        }

        static auto surface() -> GPU::Surface&
        {
            return s_renderer->m_surface;
        }

        static auto gpu_instance() -> GPU::Instance&
        {
            return s_renderer->m_instance;
        }

        [[nodiscard]]
        static auto default_material() -> AssetRef<PbrMaterial>
        {
            return s_renderer->m_default_material;
        }

        [[nodiscard]]
        static auto default_normal_map() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto white_texture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto black_texture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto white_cube_texture() -> GPU::Texture;

        void create_default_resources();
        static bool has_pipeline_statistics();

    private:
        static Renderer* s_renderer;

        AssetRef<PbrMaterial> m_default_material;
        AssetRef<Texture2D> m_white_texture, m_black_texture, m_normal_map;
        GPU::Texture m_white_cube_texture;

        Vector2 m_window_size {};
        bool m_skip_render {};

        GPU::Instance m_instance {};
        GPU::Device m_device {};
        GPU::Adapter m_adapter {};
        GPU::Surface m_surface {};
        bool m_has_pipeline_statistics {};
    };
}
