#pragma once
#include "Fussion/RHI/Instance.h"

#include "volk.h"

namespace Fussion::RHI {
class VulkanInstance : public Instance {
public:
    explicit VulkanInstance(const Window& window);
    ~VulkanInstance() override = default;

    bool CheckValidationLayers();

    VkInstance Instance{};
    VkSurfaceKHR Surface{};
};
}
