#pragma once
#include <Fussion/RHI/Device.h>
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/PbrMaterial.h>
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class Renderer {
    public:
        static void Initialize(Window const& window);
        static void Shutdown();

        static Renderer& Inst() { return *s_Renderer; }

        static auto Begin() -> Maybe<GPU::TextureView>;
        static void End(GPU::CommandBuffer cmd);
        static void Resize(Vector2 const& new_size);

        static auto Device() -> GPU::Device&
        {
            return s_Renderer->m_Device;
        }

        static auto Surface() -> GPU::Surface&
        {
            return s_Renderer->m_Surface;
        }

        [[nodiscard]]
        static auto GetDefaultMaterial() -> AssetRef<PbrMaterial> { return s_Renderer->m_DefaultMaterial; }

        [[nodiscard]]
        static auto DefaultNormalMap() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto WhiteTexture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto BlackTexture() -> AssetRef<Texture2D>;

        void CreateDefaultResources();

    private:
        static Renderer* s_Renderer;

        AssetRef<PbrMaterial> m_DefaultMaterial;
        AssetRef<Texture2D> m_WhiteTexture, m_BlackTexture, m_NormalMap;

        Vector2 m_WindowSize{};
        bool m_SkipRender{};

        GPU::Instance m_Instance{};
        GPU::Device m_Device{};
        GPU::Adapter m_Adapter{};
        GPU::Surface m_Surface{};
    };
}
