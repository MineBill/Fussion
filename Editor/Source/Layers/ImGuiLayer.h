#pragma once
#include "Engin5/Core/Layer.h"
#include "Engin5/Renderer/CommandBuffer.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class ImGuiLayer: public Engin5::Layer
{
public:
    ImGuiLayer();

    static ImGuiLayer* This() { return s_Instance; }

    void Init();
    void OnStart() override;
    void OnUpdate(f32) override;

    void Begin();
    void End(const Ref<Engin5::CommandBuffer>&);

    // @Todo The VkDescriptorSet could be changed to void*.
    std::map<u64, VkDescriptorSet> ImageToVkSet;
private:
    static ImGuiLayer* s_Instance;
};

#define IMGUI_IMAGE(image) ImGuiLayer::This()->ImageToVkSet[transmute(u64, image->GetRenderHandle<VkImage>())]