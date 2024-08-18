#include "EditorPCH.h"
#define VK_NO_PROTOTYPES
#include "ImGuiLayer.h"

#include "Fussion/RHI/Device.h"
#include "Fussion/Core/Application.h"
#include "Vulkan/VulkanDevice.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "ImGuizmo.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <tracy/Tracy.hpp>

#include "Editor.h"
#include "EditorStyle.h"
#include "Fussion/RHI/Renderer.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
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

void ImGuiLayer::LoadFonts()
{
    auto& io = ImGui::GetIO();
    auto& style = EditorStyle::GetStyle();
    using enum EditorFont;
    style.Fonts[RegularNormal] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 15.0f);
    style.Fonts[RegularBig] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 18.0f);
    style.Fonts[RegularSmall] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 13.0f);
    style.Fonts[RegularHuge] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 24.0f);
    style.Fonts[Bold] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Bold.ttf", 15.0f);
    style.Fonts[BoldSmall] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Bold.ttf", 12.0f);

    io.FontDefault = style.Fonts[RegularNormal];
}

void* ToImGuiTexture(Ref<Fussion::RHI::Image> const& image) {
    return ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, image->GetRenderHandle<VkImage>())];
}

void* ToImGuiTexture(Ref<Fussion::RHI::ImageView> const& view) {
    return ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, view->GetRawHandle())];
}

void ImGuiLayer::Init()
{
    ZoneScoped;
    using namespace Fussion;
    using namespace Fussion::RHI;
    LOG_DEBUGF("Initializing ImGui layer.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();

    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
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
        .Queue = *device->GraphicsQueue.UnsafePtr(),
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
    Device::Instance()->RegisterImageCallback([this](Ref<RHI::Image> const& image, bool create) {
        std::lock_guard lock(m_RegistrationMutex);
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
                    ImGui_ImplVulkan_RemoveTexture(TRANSMUTE(VkDescriptorSet, ImageToVkSet[handle]));
            }
        }
    });

    Device::Instance()->RegisterImageViewCallback([this](Ref<ImageView> const& view, Ref<RHI::Image> const& image, bool create) {
        std::lock_guard lock(m_RegistrationMutex);
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

                ImageToVkSet[TRANSMUTE(u64, view->GetRawHandle())] = ImGui_ImplVulkan_AddTexture(
                    vk_image->Sampler->GetRenderHandle<VkSampler>(),
                    view->GetRenderHandle<VkImageView>(), layout);

            } else {
                auto handle = TRANSMUTE(u64, view->GetRawHandle());
                if (ImageToVkSet.contains(handle))
                    ImGui_ImplVulkan_RemoveTexture(TRANSMUTE(VkDescriptorSet, ImageToVkSet[handle]));
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

void ImGuiLayer::End(const Ref<Fussion::RHI::CommandBuffer>& cmd)
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
    style.DockingSeparatorSize = 0;
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.26f, 0.26f, 0.26f, 0.66f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.97f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
    colors[ImGuiCol_Header] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 0.39f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);

}
