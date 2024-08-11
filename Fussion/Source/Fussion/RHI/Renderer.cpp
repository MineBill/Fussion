#include "e5pch.h"
#include "Renderer.h"

#include "Fussion/Util/TextureImporter.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Assets/PbrMaterial.h"

namespace Fussion::RHI {
    Renderer* Renderer::s_Renderer = nullptr;

    Renderer::~Renderer()
    {
        LOG_DEBUGF("Destroying renderer!");
        Device::s_Instance->~Device();
        Device::s_Instance = nullptr;
    }

    void Renderer::Init(Window const& window)
    {
        s_Renderer = new Renderer();

        auto instance = Instance::Create(window);
        auto* device = Device::Create(instance);
        Device::s_Instance.reset(device);
    }

    void Renderer::Shutdown()
    {
        LOG_DEBUGF("Shutdown renderer!");
        s_Renderer->~Renderer();
        s_Renderer = nullptr;
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
        return { s_Renderer->m_CommandBuffers.at(image), image };
    }

    void Renderer::End(Ref<CommandBuffer> const& cmd)
    {
        s_Renderer->m_Swapchain->SubmitCommandBuffer(cmd);
        s_Renderer->m_Swapchain->Present(s_Renderer->m_CurrentImage);
    }

    auto Renderer::DefaultNormalMap() -> AssetRef<Texture2D> { return s_Renderer->m_NormalMap; }

    auto Renderer::WhiteTexture() -> AssetRef<Texture2D> { return s_Renderer->m_WhiteTexture; }

    auto Renderer::BlackTexture() -> AssetRef<Texture2D> { return s_Renderer->m_BlackTexture; }

    void Renderer::CreateDefaultRenderpasses()
    {
        auto device = Device::Instance();
        auto main_rp_spec = RenderPassSpecification{
            .Label = "Main RenderPass",
            .Attachments = {
                RenderPassAttachment{
                    .Label = "Color Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .StoreOp = RenderPassAttachmentStoreOp::Store,
                    .Format = ImageFormat::B8G8R8A8_UNORM,
                    .FinalLayout = ImageLayout::PresentSrc,
                    .ClearColor = { 0.2f, 0.6f, 0.15f, 1.0f },
                },
                RenderPassAttachment{
                    .Label = "Depth Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .Format = ImageFormat::D32_SFLOAT,
                    .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                    .ClearDepth = 1.f,
                }
            },
            .SubPasses = {
                RenderPassSubPass{
                    .ColorAttachments = {
                        {
                            .Attachment = 0,
                            .Layout = ImageLayout::ColorAttachmentOptimal,
                        }
                    },
                    .DepthStencilAttachment = RenderPassAttachmentRef{
                        .Attachment = 1,
                        .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                    }
                },
            }
        };

        m_MainRenderPass = device->CreateRenderPass(main_rp_spec);

        auto ui_rp_spec = RenderPassSpecification{
            .Label = "UI RenderPass",
            .Attachments = {
                RenderPassAttachment{
                    .Label = "Color Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .StoreOp = RenderPassAttachmentStoreOp::Store,
                    .Format = ImageFormat::B8G8R8A8_UNORM,
                    .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                    .ClearColor = { 0.2f, 0.6f, 0.15f, 1.0f },
                },
                RenderPassAttachment{
                    .Label = "Depth Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .Format = ImageFormat::D32_SFLOAT,
                    .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                    .ClearDepth = 1.f,
                }
            },
            .SubPasses = {
                RenderPassSubPass{
                    .ColorAttachments = {
                        {
                            .Attachment = 0,
                            .Layout = ImageLayout::ColorAttachmentOptimal,
                        }
                    },
                    .DepthStencilAttachment = RenderPassAttachmentRef{
                        .Attachment = 1,
                        .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                    }
                },
            }
        };

        m_UIRenderPass = device->CreateRenderPass(ui_rp_spec);

        auto& window = Application::Instance()->GetWindow();
        auto swapchain_spec = SwapChainSpecification{
            .Size = { CAST(f32, window.GetWidth()), CAST(f32, window.GetHeight()) },
            .PresentMode = VideoPresentMode::Immediate,
            .Format = ImageFormat::B8G8R8A8_UNORM,
        };
        m_Swapchain = device->CreateSwapchain(m_MainRenderPass, swapchain_spec);

        auto cmd_spec = CommandBufferSpecification{
            .Label = "Renderer Command Buffers",
        };
        m_CommandBuffers = device->CreateCommandBuffers(m_Swapchain->GetImageCount(), cmd_spec);

    }

    static unsigned char g_white_texture_png[] = {
#include "white_texture.png.h"
    };

    static unsigned char g_black_texture_png[] = {
#include "black_texture.png.h"
    };

    static unsigned char g_normal_map_png[] = {
#include "default_normal_map.png.h"
    };

    void Renderer::CreateDefaultResources()
    {
        auto material = MakeRef<PbrMaterial>();
        material->ObjectColor = Color(1, 1, 1, 1);
        s_Renderer->m_DefaultMaterial = AssetManager::CreateVirtualAssetRef<PbrMaterial>(material);

        s_Renderer->m_WhiteTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_white_texture_png), "Default White Texture");
        s_Renderer->m_BlackTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_black_texture_png), "Default Black Texture");
        s_Renderer->m_NormalMap = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_normal_map_png, true), "Default Normal Map");

    }
}
