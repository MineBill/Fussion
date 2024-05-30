#pragma once
#include "Engin5/Renderer/Instance.h"

#include "volk.h"

namespace Engin5
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
