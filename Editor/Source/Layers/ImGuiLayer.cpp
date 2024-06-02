#include "e5pch.h"
#include "ImGuiLayer.h"

#include "Engin5/Renderer/Device.h"
#include "Engin5/Core/Application.h"
#include "Vulkan/VulkanDevice.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Engin5/Renderer/Renderer.h"

#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanImageView.h"

ImGuiLayer* ImGuiLayer::s_Instance;

static inline const char* string_VkResult(VkResult input_value) {
    switch (input_value) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED:
            return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
            return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR:
            return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR:
            return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR:
            return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR:
            return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR:
            return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
            return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
            return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
        default:
            return "Unhandled VkResult";
    }
}

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
    using namespace Engin5;
    LOG_DEBUGF("Initializing ImGui layer.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

#if !defined(OS_LINUX)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

    auto device = Device::Instance()->As<VulkanDevice>();
    auto window = cast(GLFWwindow*, Application::Instance()->GetWindow()->NativeHandle());

    ImGui_ImplGlfw_InitForVulkan(window, true);

    auto pool_spec = ResourcePoolSpecification{
        .MaxSets = 100,
        .ResourceLimits = {
            ResourceLimit{ResourceType::StorageBuffer, 100},
            ResourceLimit{ResourceType::CombinedImageSampler, 100},
            ResourceLimit{ResourceType::UniformBuffer, 100},
            ResourceLimit{ResourceType::InputAttachment, 100},
            ResourceLimit{ResourceType::Sampler, 100},
        }
    };
    auto imgui_pool = device->CreateResourcePool(pool_spec);
    auto info = ImGui_ImplVulkan_InitInfo {
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
        return vkGetInstanceProcAddr(cast(VulkanDevice*, device)->Instance->Instance, function_name);
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

                ImageToVkSet[transmute(u64, vk_image->GetRawHandle())] = ImGui_ImplVulkan_AddTexture(
                    vk_image->Sampler->GetRenderHandle<VkSampler>(),
                    vk_image->View->GetRenderHandle<VkImageView>(), layout);

            } else {
                auto handle = transmute(u64, image->GetRawHandle());
                if (ImageToVkSet.contains(handle))
                    ImGui_ImplVulkan_RemoveTexture(ImageToVkSet[handle]);
            }
        }
    });
}

void ImGuiLayer::OnStart()
{
}

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
}

void ImGuiLayer::End(const Ref<Engin5::CommandBuffer>& cmd)
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