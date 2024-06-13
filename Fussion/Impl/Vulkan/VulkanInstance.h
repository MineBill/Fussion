#pragma once
#include "Fussion/Renderer/Instance.h"

#include "volk.h"

namespace Fussion
{
    class VulkanInstance: public Instance
    {
    public:
        explicit VulkanInstance(const Window& window);
        ~VulkanInstance() override = default;

        bool CheckValidationLayers();

        VkInstance Instance{};
        VkSurfaceKHR Surface{};
    };
}
