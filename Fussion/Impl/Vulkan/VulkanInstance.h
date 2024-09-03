#pragma once
#include "Fussion/RHI/Instance.h"

#include "volk.h"

namespace Fussion::RHI {
    class VulkanInstance final : public Instance {
    public:
        explicit VulkanInstance(Window const& window);
        virtual ~VulkanInstance() override;

        auto CheckValidationLayers() -> Maybe<std::vector<const char*>>;

        static auto CreateDebugMessenger() -> VkDebugUtilsMessengerCreateInfoEXT;
        virtual void* GetRawHandle() override { return Instance; }

        VkDebugUtilsMessengerEXT DebugMessenger{};
        VkInstance Instance{};
        VkSurfaceKHR Surface{};
    };
}
