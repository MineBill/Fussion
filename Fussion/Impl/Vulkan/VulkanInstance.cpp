#include "e5pch.h"
#include "VulkanInstance.h"
#include "Common.h"
#include "Fussion/Log/Log.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Core/Core.h"

#include <GLFW/glfw3.h>
#include <cstring>

const char* g_RequiredVulkanLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char *> GetRequiredExtensions() {
    u32 count;
    auto required_extensions = glfwGetRequiredInstanceExtensions(&count);
    if (required_extensions == nullptr) {
        LOG_ERROR("Call to glfwGetRequiredInstanceExtensions failed");
    }
    std::vector<const char *> retval(count);
    for (u32 i = 0; i < count; i++) {
        retval[i] = required_extensions[i];
    }

    retval.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return retval;
}

namespace Fussion
{
    VkBool32 DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
        void *p_user_data) {
        switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR(p_callback_data->pMessage);
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

    Instance* Instance::Create(const Window& window)
    {
        return new VulkanInstance(window);
    }

    VulkanInstance::VulkanInstance(const Window& window)
    {
        VK_CHECK(volkInitialize())

        constexpr auto application_info = VkApplicationInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Fuck",
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName = "Fussion",
            .engineVersion = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion = VK_API_VERSION_1_3,
        };

        const auto extensions = GetRequiredExtensions();

        auto instance_create_info = VkInstanceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &application_info,
            .enabledExtensionCount = cast(u32, extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

#define USE_VALIDATION_LAYERS
#ifdef USE_VALIDATION_LAYERS
        CheckValidationLayers();
        LOG_INFO("Setting up validation layers and debug messenger");
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = DebugCallback,
        };

        instance_create_info.enabledLayerCount = 1;
        instance_create_info.ppEnabledLayerNames = g_RequiredVulkanLayers;
        instance_create_info.pNext = &debugUtilsMessengerCreateInfoExt;
#endif

        VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &Instance))
        volkLoadInstance(Instance);

        VK_CHECK(glfwCreateWindowSurface(Instance, cast(GLFWwindow*, window.NativeHandle()), nullptr, &Surface));
    }

    bool VulkanInstance::CheckValidationLayers()
    {
        u32 count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> properties;
        properties.resize(count);

        vkEnumerateInstanceLayerProperties(&count, properties.data());

        for (const auto required : g_RequiredVulkanLayers) {
            bool found{false};

            for (const auto& prop : properties) {
                if (strcmp(required, prop.layerName) == 0) {
                    found = true;
                }
            }

            if (!found) {
                LOG_ERRORF("Required validation layer {} not found", required);
                return false;
            }
        }

        return true;
    }

}