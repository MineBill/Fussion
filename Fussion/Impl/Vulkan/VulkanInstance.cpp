#include "FussionPCH.h"
#include "VulkanInstance.h"
#include "Common.h"
#include "Fussion/Log/Log.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Core/Core.h"

#include <GLFW/glfw3.h>
#include <cpptrace/cpptrace.hpp>

#include <cstring>

enum class LayerRequirement {
    Needed,
    Optional,
};

std::unordered_map<const char*, LayerRequirement> g_RequiredVulkanLayers = {
    { "VK_LAYER_KHRONOS_validation", LayerRequirement::Optional }
};

std::vector<const char*> GetRequiredExtensions()
{
    u32 count;
    auto required_extensions = glfwGetRequiredInstanceExtensions(&count);
    if (required_extensions == nullptr) {
        LOG_ERROR("Call to glfwGetRequiredInstanceExtensions failed");
        return {};
    }
    std::vector<const char*> retval(count);
    for (u32 i = 0; i < count; i++) {
        retval[i] = required_extensions[i];
    }

    retval.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return retval;
}

namespace Fussion::RHI {
    VkBool32 VkDebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_types,
        VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
        [[maybe_unused]] void* p_user_data)
    {
        switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            std::string message(p_callback_data->pMessage);

            // :)
            if (message.contains("EOSOverlayVkLayer"))
                break;

            LOG_ERROR(p_callback_data->pMessage);
            LOG_ERRORF("{}", cpptrace::generate_trace(1).to_string(true));
        }
        break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN(p_callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_DEBUG(p_callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO(p_callback_data->pMessage);
            break;
        default:
            break;
        }
        return true;
    }

    Ref<Instance> Instance::Create(Window const& window)
    {
        return make_ref<VulkanInstance>(window);
    }

    VulkanInstance::VulkanInstance(Window const& window)
    {
        VK_CHECK(volkInitialize())

        constexpr auto application_info = VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Fussion Project",
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName = "Fussion",
            .engineVersion = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion = VK_API_VERSION_1_3,
        };

        auto extensions = GetRequiredExtensions();

        auto instance_create_info = VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &application_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = CAST(u32, extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

#define USE_VALIDATION_LAYERS
#ifdef USE_VALIDATION_LAYERS
        auto found_layers = CheckValidationLayers();
        if (!found_layers) {
            PANIC("A required validation layer was not found");
        }
        LOG_INFO("Setting up validation layers and debug messenger");
        auto debug_utils_messenger_create_info_ext = CreateDebugMessenger();

        instance_create_info.enabledLayerCount = CAST(u32, found_layers->size());
        instance_create_info.ppEnabledLayerNames = found_layers->data();
        instance_create_info.pNext = &debug_utils_messenger_create_info_ext;
#endif

        VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &Instance))
        volkLoadInstance(Instance);

        vkCreateDebugUtilsMessengerEXT(Instance, &debug_utils_messenger_create_info_ext, nullptr, &DebugMessenger);

        VK_CHECK(glfwCreateWindowSurface(Instance, CAST(GLFWwindow*, window.native_handle()), nullptr, &Surface));
    }

    VulkanInstance::~VulkanInstance()
    {
        LOG_DEBUGF("Destroying vulkan instance");
    }

    auto VulkanInstance::CheckValidationLayers() -> Maybe<std::vector<const char*>>
    {
        std::vector<const char*> found_layers{};
        u32 count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> properties;
        properties.resize(count);

        vkEnumerateInstanceLayerProperties(&count, properties.data());

        for (auto [name, requirement] : g_RequiredVulkanLayers) {
            bool found{ false };

            for (auto const& prop : properties) {
                if (strcmp(name, prop.layerName) == 0) {
                    found = true;
                }
            }

            if (!found) {
                switch (requirement) {
                case LayerRequirement::Needed:
                    LOG_ERRORF("Required validation layer {} not found", name);
                    return None();
                case LayerRequirement::Optional:
                    LOG_WARNF("Optional validation layer {} not found", name);
                    break;
                }
            } else {
                found_layers.push_back(name);
            }
        }

        return found_layers;
    }

    auto VulkanInstance::CreateDebugMessenger() -> VkDebugUtilsMessengerCreateInfoEXT
    {
        return VkDebugUtilsMessengerCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = {},
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = VkDebugMessengerCallback,
            .pUserData = nullptr,
        };
    }

}
