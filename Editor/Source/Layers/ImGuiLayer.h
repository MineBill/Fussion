#pragma once
#include "Fussion/Core/Layer.h"
#include "Fussion/RHI/CommandBuffer.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

class ImGuiLayer : public Fussion::Layer {
public:
    ImGuiLayer();

    static ImGuiLayer* This() { return s_Instance; }

    void Init();
    virtual void OnStart() override;
    virtual void OnUpdate(f32) override;

    void Begin();
    void End(Ref<Fussion::RHI::CommandBuffer> const&);

    // @Todo The VkDescriptorSet could be changed to void*.
    std::map<u64, VkDescriptorSet> ImageToVkSet;

private:
    static ImGuiLayer* s_Instance;
};

// @todo This should probably be replaced with a function??
#define IMGUI_IMAGE(image) ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, image->GetRenderHandle<VkImage>())]

inline void* ToImGuiTexture(Ref<Fussion::RHI::Image> const& image)
{
    return ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, image->GetRenderHandle<VkImage>())];
}

inline void* ToImGuiTexture(Ref<Fussion::RHI::ImageView> const& view)
{
    return ImGuiLayer::This()->ImageToVkSet[TRANSMUTE(u64, view->GetRawHandle())];
}
