#include "e5pch.h"
#include "VulkanDevice.h"

#include "Common.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanPipepline.h"
#include "VulkanRenderPass.h"
#include "VulkanSampler.h"
#include "VulkanShader.h"
#include "VulkanSwapchain.h"
#include "Resources/VulkanResource.h"
#include "Resources/VulkanResourcePool.h"
#include <cstring>

const char* g_RequiredDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

namespace Fussion::RHI {
Device* Device::Create(RHI::Instance* instance)
{
    return new VulkanDevice(instance);
}

std::vector<u32> QueueFamilyIndices::GetUniqueIndex() const
{
    if (*GraphicsFamily == *PresentFamily) {
        return { *GraphicsFamily };
    }
    LOG_ERROR("Present and Graphics indices differ, do something");
    return {};
}

VulkanDevice::VulkanDevice(RHI::Instance* instance)
{
    Instance = dynamic_cast<VulkanInstance*>(instance);

    SetupDebugCallback();
    PickPhysicalDevice();

    FamilyIndices = GetQueueFamilies(PhysicalDevice);

    CreateLogicalDevice();
    CreateCommandPool();

    auto const functions = VmaVulkanFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2,
        .vkBindBufferMemory2KHR = vkBindBufferMemory2,
        .vkBindImageMemory2KHR = vkBindImageMemory2,
        .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
        .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
    };

    auto allocator_create_info = VmaAllocatorCreateInfo{
        .physicalDevice = PhysicalDevice,
        .device = Handle,
        .pVulkanFunctions = &functions,
        .instance = Instance->Instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };

    VK_CHECK(vmaCreateAllocator(&allocator_create_info, &Allocator))
}

VulkanDevice::~VulkanDevice()
{
    LOG_INFO("Destroying vulkan device");
}

Ref<RenderPass> VulkanDevice::CreateRenderPass(RenderPassSpecification spec)
{
    return std::make_shared<VulkanRenderPass>(this, spec);
}

auto VulkanDevice::CreateCommandBuffer(CommandBufferSpecification spec) -> Ref<CommandBuffer>
{
    return std::make_shared<VulkanCommandBuffer>(this, spec);
}

auto VulkanDevice::CreateCommandBuffers(s32 count, CommandBufferSpecification spec) -> std::vector<Ref<CommandBuffer>>
{
    std::vector<Ref<CommandBuffer>> ret;
    for (s32 i = 0; i < count; i++) {
        ret.push_back(CreateCommandBuffer(spec));
    }
    return ret;
}

auto VulkanDevice::CreateSampler(SamplerSpecification spec) -> Ref<Sampler>
{
    return std::make_shared<VulkanSampler>(this, spec);
}

auto VulkanDevice::CreateImage(ImageSpecification spec) -> Ref<Image>
{
    auto image = MakeRef<VulkanImage>(this, spec);

    for (auto const& cb : m_ImageCallbacks) {
        cb(image, true);
    }

    return image;
}

auto VulkanDevice::CreateImageView(Ref<Image> image, ImageViewSpecification spec) -> Ref<ImageView>
{
    auto view = MakeRef<VulkanImageView>(this, dynamic_cast<VulkanImage*>(image.get()), spec);

    for (auto const& cb : m_ImageViewCallbacks) {
        cb(view, image, true);
    }

    return view;
}

auto VulkanDevice::CreateSwapchain(Ref<RenderPass> render_pass, SwapChainSpecification spec) -> Ref<Swapchain>
{
    return std::make_shared<VulkanSwapchain>(this, render_pass, spec);
}

auto VulkanDevice::CreateFrameBuffer(Ref<RenderPass> render_pass, FrameBufferSpecification spec) -> Ref<FrameBuffer>
{
    return std::make_shared<VulkanFrameBuffer>(this, render_pass, spec);
}

auto VulkanDevice::CreateFrameBufferFromImages(
    Ref<RenderPass> render_pass,
    std::vector<Ref<Image>> images,
    FrameBufferSpecification spec) -> Ref<FrameBuffer>
{
    return std::make_shared<VulkanFrameBuffer>(this, render_pass, images, spec);
}

auto VulkanDevice::CreateFrameBufferFromImageViews(
    Ref<RenderPass> render_pass,
    std::vector<Ref<ImageView>> images,
    FrameBufferSpecification spec) -> Ref<FrameBuffer>
{
    return MakeRef<VulkanFrameBuffer>(this, render_pass, images, spec);
}

auto VulkanDevice::CreateResourceLayout(std::span<ResourceUsage> resources) -> Ref<ResourceLayout>
{
    return std::make_shared<VulkanResourceLayout>(this, resources);
}

auto VulkanDevice::CreateBuffer(BufferSpecification spec) -> Ref<Buffer>
{
    return MakeRef<VulkanBuffer>(this, spec);
}

auto VulkanDevice::CreatePipelineLayout(std::vector<Ref<ResourceLayout>> const layouts, PipelineLayoutSpecification spec) -> Ref<PipelineLayout>
{
    return VulkanPipelineLayout::Create(this, layouts, spec);
}

auto VulkanDevice::CreatePipeline(
    Ref<RenderPass> const& render_pass,
    Ref<Shader> const& shader,
    Ref<PipelineLayout> const& layout,
    const PipelineSpecification& spec) -> Ref<Pipeline>
{
    return VulkanPipeline::Create(this, shader, layout, render_pass, spec);
}

auto VulkanDevice::CreateShader(Ref<RenderPass> render_pass, std::span<ShaderStage> stages, ShaderMetadata metadata) -> Ref<Shader>
{
    return VulkanShader::Create(this, render_pass, stages, metadata);
}

auto VulkanDevice::CreateResourcePool(ResourcePoolSpecification spec) -> Ref<ResourcePool>
{
    return MakeRef<VulkanResourcePool>(this, spec);
}

void VulkanDevice::WaitIdle()
{
    VK_CHECK(vkDeviceWaitIdle(Handle))
}

auto VulkanDevice::BeginSingleTimeCommand() -> Ref<CommandBuffer>
{
    auto cmd = CreateCommandBuffer({
        .Label = "Single Time Command"
    });
    cmd->Begin();
    return cmd;
}

void VulkanDevice::EndSingleTimeCommand(Ref<CommandBuffer> cmd)
{
    auto const vk_cmd = dynamic_cast<VulkanCommandBuffer*>(cmd.get());
    vk_cmd->End(CommandBufferType::None);

    auto submit_info = VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk_cmd->Handle,
    };

    vkQueueSubmit(GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(GraphicsQueue);
}

auto VulkanDevice::CreateFence(bool signaled) const -> VkFence
{
    auto ci = VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if (signaled) {
        ci.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VkFence handle;
    VK_CHECK(vkCreateFence(Handle, &ci, nullptr, &handle))
    return handle;
}

auto VulkanDevice::CreateSemaphore() const -> VkSemaphore
{
    auto ci = VkSemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore handle;
    VK_CHECK(vkCreateSemaphore(Handle, &ci, nullptr, &handle))
    return handle;
}

auto VulkanDevice::CreateFences(s32 count, bool signaled) const -> std::vector<VkFence>
{
    std::vector<VkFence> ret;
    for (s32 i = 0; i < count; i++) {
        ret.push_back(CreateFence(signaled));
    }
    return ret;
}

auto VulkanDevice::CreateSemaphores(s32 count) const -> std::vector<VkSemaphore>
{
    std::vector<VkSemaphore> ret;
    for (s32 i = 0; i < count; i++) {
        ret.push_back(CreateSemaphore());
    }
    return ret;
}

void VulkanDevice::DestroyFences(std::span<VkFence> fences)
{
    auto const device = Device::Instance()->As<VulkanDevice>();
    for (auto const fence : fences) {
        vkDestroyFence(device->Handle, fence, nullptr);
    }
}

void VulkanDevice::DestroySemaphores(std::span<VkSemaphore> semaphores)
{
    auto const device = Device::Instance()->As<VulkanDevice>();
    for (auto const semaphore : semaphores) {
        vkDestroySemaphore(device->Handle, semaphore, nullptr);
    }
}

void VulkanDevice::SetHandleName(u64 handle, VkObjectType type, const std::string& name)
{
    auto name_info = VkDebugUtilsObjectNameInfoEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name.c_str()
    };

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(Handle, &name_info))
}

void VulkanDevice::SetupDebugCallback() {}

void VulkanDevice::PickPhysicalDevice()
{
    u32 count;
    vkEnumeratePhysicalDevices(Instance->Instance, &count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(count);

    vkEnumeratePhysicalDevices(Instance->Instance, &count, physical_devices.data());

    for (auto const device : physical_devices) {
        if (IsDeviceSuitable(device)) {
            PhysicalDevice = device;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            LOG_INFOF("Selected GPU: {}", properties.deviceName);
            LOG_INFOF("\tDriver version: {}.{}.{}", VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));
            LOG_INFOF("\tAPI version: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
            return;
        }
    }
    LOG_FATAL("Could not find a suitable GPU device");
}

void VulkanDevice::CreateLogicalDevice()
{
    auto const unique_families = FamilyIndices.GetUniqueIndex();

    std::vector<VkDeviceQueueCreateInfo> queue_infos{};

    for (auto const& family : unique_families) {
        constexpr f32 queue_priority[]{ 1.0f };
        queue_infos.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = family,
            .queueCount = 1,
            .pQueuePriorities = queue_priority,
        });
    }

    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT }; // Partial binding is required for descriptor aliasing.
    descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;

    VkDeviceCreateInfo const device_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &descriptor_indexing_features,
        .queueCreateInfoCount = CAST(u32, queue_infos.size()),
        .pQueueCreateInfos = queue_infos.data(),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = g_RequiredDeviceExtensions,
        .pEnabledFeatures = nullptr,
    };

    VK_CHECK(vkCreateDevice(PhysicalDevice, &device_create_info, nullptr, &Handle))

    vkGetDeviceQueue(Handle, *FamilyIndices.GraphicsFamily, 0, &GraphicsQueue);
    vkGetDeviceQueue(Handle, *FamilyIndices.PresentFamily, 0, &PresentQueue);
}

void VulkanDevice::CreateCommandPool()
{
    auto const pool_create_info = VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = CAST(u32, FamilyIndices.GraphicsFamily.value()),
    };

    VK_CHECK(vkCreateCommandPool(Handle, &pool_create_info, nullptr, &CommandPool))
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    u32 count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> extension_properties;
    extension_properties.resize(count);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extension_properties.data());

    for (auto& required_device_extension : g_RequiredDeviceExtensions) {
        bool found = false;
        for (auto property : extension_properties) {
            if (strcmp(required_device_extension, property.extensionName) == 0) {
                found = true;
            }
        }

        if (!found) {
            LOG_ERRORF("Required device extension '{}' not found.", required_device_extension);
            return false;
        }
    }
    return true;
}

bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
{
    auto const indices = GetQueueFamilies(device);
    bool const extensions_supported = CheckDeviceExtensionSupport(device);

    bool swapchain_good = false;
    if (extensions_supported) {
        SwapchainSupport = QuerySwapChainSupport(device);
        swapchain_good = !SwapchainSupport.PresentModes.empty() && !SwapchainSupport.Formats.empty();
    }
    return indices.IsComplete() && extensions_supported && swapchain_good;
}

QueueFamilyIndices VulkanDevice::GetQueueFamilies(VkPhysicalDevice device) const
{
    u32 count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> family_properties;
    family_properties.resize(count);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, family_properties.data());

    QueueFamilyIndices indices;
    u32 index = 0;
    for (auto property : family_properties) {
        if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.GraphicsFamily = index;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, Instance->Surface, &present_support);
        if (present_support) {
            indices.PresentFamily = index;
        }
        if (indices.IsComplete()) {
            break;
        }
        index++;
    }

    return indices;
}

auto VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device) const -> SwapChainSupportDetails
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Instance->Surface, &details.Capabilities);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, Instance->Surface, &format_count, nullptr);

    if (format_count != 0) {
        details.Formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, Instance->Surface, &format_count, details.Formats.data());
    }

    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, Instance->Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, Instance->Surface, &presentModeCount, details.PresentModes.data());
    }
    return details;
}

void VulkanDevice::RegisterImageCallback(VulkanDevice::ImageCreationCallback const& callback)
{
    m_ImageCallbacks.push_back(callback);
}

void VulkanDevice::RegisterImageViewCallback(ImageViewCreationCallback const& callback)
{
    m_ImageViewCallbacks.push_back(callback);
}

void VulkanDevice::DestroyImage(Ref<Image> const& image)
{
    for (auto const& cb : m_ImageCallbacks) {
        cb(image, false);
    }
    image->Destroy();
}

void VulkanDevice::DestroyImageView(Ref<ImageView> const& image_view)
{
    if (image_view) {
        for (auto const& cb : m_ImageViewCallbacks) {
            cb(image_view, nullptr, false);
        }
        vkDestroyImageView(Handle, image_view->GetRenderHandle<VkImageView>(), nullptr);
    }
}

void VulkanDevice::DestroySampler(Ref<Sampler> const& sampler)
{
    if (sampler)
        vkDestroySampler(Handle, sampler->GetRenderHandle<VkSampler>(), nullptr);
}

void VulkanDevice::DestroyFrameBuffer(Ref<Image> const& frame_buffer)
{
    if (frame_buffer)
        vkDestroyFramebuffer(Handle, frame_buffer->GetRenderHandle<VkFramebuffer>(), nullptr);
}
}
