#include "e5pch.h"
#include "Renderer.h"

#include "Engin5/Core/Application.h"

namespace Engin5
{
    Renderer* Renderer::s_Renderer = nullptr;

    void Renderer::Init(const Window& window)
    {
        s_Renderer = new Renderer();

        const auto instance = Instance::Create(window);
        auto* device = Device::Create(instance);
        Device::s_Instance.reset(device);
    }

    Renderer* Renderer::GetInstance()
    {
        return s_Renderer;
    }

    auto Renderer::Begin() -> std::tuple<Ref<CommandBuffer>, u32>
    {
        auto [image, ok] = s_Renderer->m_Swapchain->GetNextImage();
        if (!ok) {
            auto& window = Application::Instance()->GetWindow();
            // @note Is there a better way to handle resizing?
            Device::Instance()->WaitIdle();
            s_Renderer->m_Swapchain->Resize(window.GetWidth(), window.GetHeight());
            return {};
        }
        s_Renderer->m_CurrentImage = image;
        return {s_Renderer->m_CommandBuffers.at(image), image};
    }

    void Renderer::End(const Ref<CommandBuffer>& cmd)
    {
        s_Renderer->m_Swapchain->SubmitCommandBuffer(cmd);
        s_Renderer->m_Swapchain->Present(s_Renderer->m_CurrentImage);
    }

    void Renderer::CreateDefaultResources()
    {
        const auto device = Device::Instance();
        const auto main_rp_spec = RenderPassSpecification {
            .Label = "Main RenderPass",
            .Attachments = {
                RenderPassAttachment {
                    .Label = "Color Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .StoreOp = RenderPassAttachmentStoreOp::Store,
                    .Format = ImageFormat::B8G8R8A8_UNORM,
                    .FinalLayout = ImageLayout::PresentSrc,
                    .ClearColor = {0.2f, 0.6f, 0.15f, 1.0f},
                },
                RenderPassAttachment {
                    .Label = "Depth Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .Format = ImageFormat::D32_SFLOAT,
                    .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                    .ClearDepth = 1.f,
                }
            },
            .SubPasses = {
                RenderPassSubPass {
                    .ColorAttachments = {
                        {
                            .Attachment = 0,
                            .Layout = ImageLayout::ColorAttachmentOptimal,
                        }
                    },
                    .DepthStencilAttachment = RenderPassAttachmentRef {
                        .Attachment = 1,
                        .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                    }
                },
            }
        };

        m_MainRenderPass = device->CreateRenderPass(main_rp_spec);

        const auto ui_rp_spec = RenderPassSpecification {
            .Label = "UI RenderPass",
            .Attachments = {
                RenderPassAttachment {
                    .Label = "Color Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .StoreOp = RenderPassAttachmentStoreOp::Store,
                    .Format = ImageFormat::B8G8R8A8_UNORM,
                    .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                    .ClearColor = {0.2f, 0.6f, 0.15f, 1.0f},
                },
                RenderPassAttachment {
                    .Label = "Depth Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .Format = ImageFormat::D32_SFLOAT,
                    .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                    .ClearDepth = 1.f,
                }
            },
            .SubPasses = {
                RenderPassSubPass {
                    .ColorAttachments = {
                        {
                            .Attachment = 0,
                            .Layout = ImageLayout::ColorAttachmentOptimal,
                        }
                    },
                    .DepthStencilAttachment = RenderPassAttachmentRef {
                        .Attachment = 1,
                        .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                    }
                },
            }
        };

        m_UIRenderPass = device->CreateRenderPass(ui_rp_spec);

        auto& window = Application::Instance()->GetWindow();
        const auto swapchain_spec = SwapChainSpecification {
            .Size = {cast(f32, window.GetWidth()), cast(f32, window.GetHeight())},
            .PresentMode = VideoPresentMode::Immediate,
            .Format = ImageFormat::B8G8R8A8_UNORM,
        };
        m_Swapchain = device->CreateSwapchain(m_MainRenderPass, swapchain_spec);


        const auto cmd_spec = CommandBufferSpecification {
            .Label = "Renderer Command Buffers",
        };
        m_CommandBuffers = device->CreateCommandBuffers(m_Swapchain->GetImageCount(), cmd_spec);
    }
}
