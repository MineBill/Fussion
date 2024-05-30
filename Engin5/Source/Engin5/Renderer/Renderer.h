#pragma once
#include "Device.h"
#include "Engin5/Window.h"

namespace Engin5
{
    class Renderer
    {
    public:
        static void Init(const Window& window);
        static Renderer* GetInstance();

        static auto Begin() -> std::tuple<Ref<CommandBuffer>, u32>;
        static void End(const Ref<CommandBuffer>& cmd);

        Ref<RenderPass> GetMainRenderPass() const { return m_MainRenderPass; }
        Ref<RenderPass> GetUIRenderPass() const { return m_UIRenderPass; }
        Ref<Swapchain> GetSwapchain() const {return m_Swapchain; }

        void CreateDefaultResources();
    private:

        static Renderer* s_Renderer;
        Ref<RenderPass> m_MainRenderPass{}, m_UIRenderPass{};
        Ref<Swapchain> m_Swapchain{};
        std::vector<Ref<CommandBuffer>> m_CommandBuffers{};
        u32 m_CurrentImage{};

        Device* m_Device{};
    };
}
