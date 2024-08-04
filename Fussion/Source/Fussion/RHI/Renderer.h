#pragma once
#include "Device.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Window.h"

namespace Fussion::RHI {
    class Renderer {
    public:
        ~Renderer();

        static void Init(Window const& window);
        static void Shutdown();
        static Renderer* GetInstance();

        static auto Begin() -> std::tuple<Ref<CommandBuffer>, u32>;
        static void End(Ref<CommandBuffer> const& cmd);

        [[nodiscard]]
        Ref<RenderPass> GetMainRenderPass() const { return m_MainRenderPass; }

        [[nodiscard]]
        Ref<RenderPass> GetUIRenderPass() const { return m_UIRenderPass; }

        [[nodiscard]]
        Ref<Swapchain> GetSwapchain() const { return m_Swapchain; }

        [[nodiscard]]
        static auto GetDefaultMaterial() -> AssetRef<PbrMaterial> { return s_Renderer->m_DefaultMaterial; }

        void CreateDefaultRenderpasses();
        void CreateDefaultResources();

    private:
        static Renderer* s_Renderer;

        Ref<RenderPass> m_MainRenderPass{}, m_UIRenderPass{};
        Ref<Swapchain> m_Swapchain{};
        std::vector<Ref<CommandBuffer>> m_CommandBuffers{};
        u32 m_CurrentImage{};

        AssetRef<PbrMaterial> m_DefaultMaterial;

        Device* m_Device{};
    };
}
