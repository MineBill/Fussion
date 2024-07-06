#include "e5pch.h"
#include "ImGuiLayer.h"

#include "Fussion/Renderer/Device.h"
#include "Fussion/Core/Application.h"
#include "Vulkan/VulkanDevice.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "ImGuizmo.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "EditorApplication.h"
#include "Editor.h"
#include "EditorStyle.h"
#include "Fussion/Renderer/Renderer.h"

#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanImageView.h"
#include "Vulkan/Common.h"

ImGuiLayer* ImGuiLayer::s_Instance;

void SetupImGuiStyle();

ImGuiLayer::ImGuiLayer()
{
    if (s_Instance != nullptr) {
        LOG_FATAL("Multiple instances of ImGuiLayer detected. Cannot proceed.");
        PANIC();
    }

    s_Instance = this;
}

void ImGuiLayer::Init()
{
    ZoneScoped;
    using namespace Fussion;
    LOG_DEBUGF("Initializing ImGui layer.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

    auto& style = Editor::Get().GetStyle();
    style.Fonts.RegularNormal = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 14.0f);
    style.Fonts.RegularSmall = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 12.0f);
    style.Fonts.RegularHuge = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 24.0f);
    style.Fonts.Bold = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Bold.ttf", 15.0f);
    io.FontDefault = style.Fonts.RegularNormal;

#if !defined(OS_LINUX)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

    auto device = Device::Instance()->As<VulkanDevice>();
    auto window = CAST(GLFWwindow*, Application::Instance()->GetWindow().NativeHandle());

    ImGui_ImplGlfw_InitForVulkan(window, true);

    auto pool_spec = ResourcePoolSpecification{
        .MaxSets = 100,
        .ResourceLimits = {
            ResourceLimit{ ResourceType::StorageBuffer, 100 },
            ResourceLimit{ ResourceType::CombinedImageSampler, 100 },
            ResourceLimit{ ResourceType::UniformBuffer, 100 },
            ResourceLimit{ ResourceType::InputAttachment, 100 },
            ResourceLimit{ ResourceType::Sampler, 100 },
        }
    };
    auto imgui_pool = device->CreateResourcePool(pool_spec);
    auto info = ImGui_ImplVulkan_InitInfo{
        .Instance = device->Instance->Instance,
        .PhysicalDevice = device->PhysicalDevice,
        .Device = device->Handle,
        .QueueFamily = 0,
        .Queue = device->GraphicsQueue,
        .DescriptorPool = imgui_pool->GetRenderHandle<VkDescriptorPool>(),
        .RenderPass = Renderer::GetInstance()->GetMainRenderPass()->GetRenderHandle<VkRenderPass>(),
        .MinImageCount = 2,
        .ImageCount = 2,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = nullptr,
        .Subpass = 0,
        .UseDynamicRendering = false,
        .Allocator = nullptr,
        .CheckVkResultFn = [](VkResult err) {
            if (err != VK_SUCCESS) {
                LOG_ERRORF("Vulkan Error from imgui: {}", string_VkResult(err));
            }
        },
    };
    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* device) {
        return vkGetInstanceProcAddr(CAST(VulkanDevice*, device)->Instance->Instance, function_name);
    }, device.get());
    ImGui_ImplVulkan_Init(&info);
    Device::Instance()->RegisterImageCallback([this](Ref<Image> const& image, bool create) {
        if (image->GetSpec().Usage.Test(ImageUsage::Sampled)) {
            if (create) {
                auto vk_image = image->As<VulkanImage>();
                VkImageLayout layout;
                auto image_layout = vk_image->GetSpec().FinalLayout;
                switch (image_layout) {
                case ImageLayout::Undefined:
                case ImageLayout::ColorAttachmentOptimal: {
                    layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
                }
                break;
                case ImageLayout::DepthStencilAttachmentOptimal: {
                    layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                }
                break;
                default: {
                    layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
                break;
                }

                ImageToVkSet[TRANSMUTE(u64, vk_image->GetRawHandle())] = ImGui_ImplVulkan_AddTexture(
                    vk_image->Sampler->GetRenderHandle<VkSampler>(),
                    vk_image->View->GetRenderHandle<VkImageView>(), layout);

            } else {
                auto handle = TRANSMUTE(u64, image->GetRawHandle());
                if (ImageToVkSet.contains(handle))
                    ImGui_ImplVulkan_RemoveTexture(ImageToVkSet[handle]);
            }
        }
    });

    SetupImGuiStyle();
}

void ImGuiLayer::OnStart() {}

void ImGuiLayer::OnUpdate(const f32 delta)
{
    (void)delta;
}

void ImGuiLayer::Begin()
{
    ZoneScoped;
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void ImGuiLayer::End(const Ref<Fussion::CommandBuffer>& cmd)
{
    ZoneScoped;
    ImGui::Render();
    auto io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // auto ctx = ImGui::GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        // ImGui::SetCurrentContext(ctx);
    }
    if (cmd != nullptr) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->GetRenderHandle<VkCommandBuffer>());
    }
}

void SetupImGuiStyle()
{
    // Fork of Photoshop style from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 2.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 2.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 13.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabMinSize = 7.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 2.0f;
    style.TabBorderSize = 1.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317f, 0.2745098173618317f, 0.2745098173618317f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3910000026226044f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6700000166893005f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288f, 0.5843137502670288f, 0.5843137502670288f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
}
