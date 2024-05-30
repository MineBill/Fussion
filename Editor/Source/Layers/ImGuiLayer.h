#pragma once
#include "Engin5/Core/Layer.h"
#include "Engin5/Renderer/CommandBuffer.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class ImGuiLayer: public Engin5::Layer
{
public:
    void OnStart() override;
    void OnUpdate() override;

    void Begin();
    void End(Ref<Engin5::CommandBuffer> cmd);

    std::map<u64, VkDescriptorSet> Sets;
};
