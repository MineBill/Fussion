#pragma once
#include "Fussion/Core/Layer.h"
#include "Fussion/Renderer/CommandBuffer.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class ImGuiLayer: public Fussion::Layer
{
public:
    ImGuiLayer();

    static ImGuiLayer* This() { return s_Instance; }

    void Init();
    void OnStart() override;
    void OnUpdate(f32) override;

    void Begin();
    void End(const Ref<Fussion::CommandBuffer>&);

    // @Todo The VkDescriptorSet could be changed to void*.
    std::map<u64, VkDescriptorSet> ImageToVkSet;
private:
    static ImGuiLayer* s_Instance;
};

// @todo This should probably be replaced with a function??
#define IMGUI_IMAGE(image) ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, image->GetRenderHandle<VkImage>())]