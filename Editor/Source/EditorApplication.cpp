#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"
#include "Layers/ImGuiLayer.h"

#include "Engin5/Renderer/Renderer.h"
#include "Engin5/Input/Input.h"
#include "imgui.h"


#include <glm/gtc/matrix_transform.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Engin5/OS/FileSystem.h"
#include "Engin5/Renderer/ShaderCompiler.h"

void EditorApplication::OnStart()
{
    using namespace Engin5;
    m_Layer = MakePtr<ImGuiLayer>();
    PushLayer(m_Layer.get());

    Application::OnStart();

    auto aspect = cast(f32, m_Window->GetWidth()) / cast(f32, m_Window->GetHeight());
    m_Perspective = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);

    m_GlobalData = MakePtr<UniformBuffer<GlobalData>>();
    m_GlobalData->GetData().Perspective = m_Perspective;
    // m_GlobalData->GetData().View = glm::translate(glm::mat4(1.0f), Vector3(1, 0, -2));
    m_GlobalData->GetData().View = glm::mat4(1.0f);

    const auto data = FileSystem::ReadEntireFile("Assets/triangle.shader");
    auto [stages, metadata] = ShaderCompiler::Compile(data);

    m_TriangleShader = Device::Instance()->CreateShader(Renderer::GetInstance()->GetMainRenderPass(), stages, metadata);

    const auto pool_spec = ResourcePoolSpecification::Default(100);
    m_ResourcePool = Device::Instance()->CreateResourcePool(pool_spec);

    std::vector resource_usages = {
        ResourceUsage{
            .Label = "awd",
            .Type = ResourceType::UniformBuffer,
            .Count = 1,
            .Stages = ShaderType::Vertex | ShaderType::Fragment,
        },
    };
    auto layout = Device::Instance()->CreateResourceLayout(resource_usages);
    auto result = m_ResourcePool->Allocate(layout);
    if (result.IsError()) {
        LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
        return;
    }

    m_GlobalResource = result.TakeValue();

    const auto scene_rp_spec = RenderPassSpecification {
        .Label = "Scene RenderPass",
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

    m_SceneRenderPass = Device::Instance()->CreateRenderPass(scene_rp_spec);

    auto fb_spec = FrameBufferSpecification {
        .Width = 400,
        .Height = 400,
        .Attachments = {
                FrameBufferAttachmentInfo {
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Usage = ImageUsage::Sampled | ImageUsage::ColorAttachment,
                },
                FrameBufferAttachmentInfo {
                        .Format = ImageFormat::D32_SFLOAT,
                        .Usage = ImageUsage::Sampled | ImageUsage::DepthStencilAttachment,
                }
        }
    };
    m_FrameBuffer = Device::Instance()->CreateFrameBuffer(m_SceneRenderPass, fb_spec);
}

bool operator==(const Vector2& vec, const Vector2& rhs)
{
    return vec.x == rhs.x && vec.y == rhs.y;
}

void EditorApplication::OnUpdate(f32 delta)
{
    using namespace Engin5;
    m_Layer->Begin();

    auto forward = Input::GetAxis(KeyboardKey::W, KeyboardKey::S);
    auto strafe = Input::GetAxis(KeyboardKey::A, KeyboardKey::D);
    m_GlobalData->GetData().View = glm::translate(m_GlobalData->GetData().View, Vector3(strafe, 0, forward) * 0.001f);

    ImGui::ShowDemoWindow();
    if (ImGui::Begin("Settings")) {
        if (ImGui::DragFloat("FOV", &m_FOV)) {
        }

        static Vector2 pos{};
        if (ImGui::DragFloat2("Position", &pos[0])) {
            m_Window->SetPosition(pos);
        }

        auto new_size = ImGui::GetContentRegionAvail();
        // if (m_ViewportSize != new_size) {
        //     Device::Instance()->WaitIdle();
        //     m_FrameBuffer->Resize(new_size);
        // }
        m_ViewportSize = new_size;
        // auto set = m_Layer->Sets[transmute(u64, m_FrameBuffer->GetColorAttachment(0)->GetRawHandle())];
        // ImGui::Image(set, m_ViewportSize);
    }
    ImGui::End();

    // const auto aspect = cast(f32, m_Window->GetWidth()) / cast(f32, m_Window->GetHeight());
    const auto aspect = m_ViewportSize.x / m_ViewportSize.y;
    m_Perspective = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 1000.0f);
    m_GlobalData->GetData().Perspective = m_Perspective;
    m_GlobalData->Flush();

    Application::OnUpdate(delta);
    auto [cmd, image] = Renderer::Begin();
    if (cmd == nullptr) {
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{cast(f32, m_Window->GetWidth()), cast(f32, m_Window->GetHeight())};

    cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffer);
        cmd->SetViewport({m_ViewportSize.x, -m_ViewportSize.y});
        cmd->SetScissor({0, 0, m_ViewportSize.x, m_ViewportSize.y});
        cmd->UseShader(m_TriangleShader);
        cmd->BindResource(m_GlobalResource, m_TriangleShader, 0);
        cmd->BindUniformBuffer(m_GlobalData->GetBuffer(), m_GlobalResource, 0);
        cmd->Draw(3, 1);
    cmd->EndRenderPass(Renderer::GetInstance()->GetUIRenderPass());

    const auto main = Renderer::GetInstance()->GetMainRenderPass();
    cmd->BeginRenderPass(main, Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    cmd->SetViewport({window_size.x, -window_size.y});
    cmd->SetScissor({0, 0, window_size.x, window_size.y});

    m_Layer->End(cmd);

    cmd->EndRenderPass(main);

    cmd->End();

    Renderer::End(cmd);
}