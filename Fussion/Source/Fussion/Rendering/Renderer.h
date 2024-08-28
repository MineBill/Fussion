#pragma once
#include <Fussion/RHI/Device.h>
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/PbrMaterial.h>

namespace Fussion {
    class Renderer {
    public:
        ~Renderer();

        static void Init(Window const& window);
        static void Shutdown();
        static Renderer* GetInstance();

        static auto Begin() -> std::tuple<Ref<RHI::CommandBuffer>, u32>;
        static void End(Ref<RHI::CommandBuffer> const& cmd);

        [[nodiscard]]
        Ref<RHI::RenderPass> GetMainRenderPass() const { return m_MainRenderPass; }

        [[nodiscard]]
        Ref<RHI::RenderPass> GetUIRenderPass() const { return m_UIRenderPass; }

        [[nodiscard]]
        Ref<RHI::Swapchain> GetSwapchain() const { return m_Swapchain; }

        [[nodiscard]]
        static auto GetDefaultMaterial() -> AssetRef<PbrMaterial> { return s_Renderer->m_DefaultMaterial; }

        [[nodiscard]]
        static auto DefaultNormalMap() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto WhiteTexture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto BlackTexture() -> AssetRef<Texture2D>;

        void CreateDefaultRenderpasses();
        void CreateDefaultResources();

    private:
        static Renderer* s_Renderer;

        Ref<RHI::RenderPass> m_MainRenderPass{}, m_UIRenderPass{};
        Ref<RHI::Swapchain> m_Swapchain{};
        std::vector<Ref<RHI::CommandBuffer>> m_CommandBuffers{};
        u32 m_CurrentImage{};

        AssetRef<PbrMaterial> m_DefaultMaterial;
        AssetRef<Texture2D> m_WhiteTexture, m_BlackTexture, m_NormalMap;

        RHI::Device* m_Device{};
    };
}
