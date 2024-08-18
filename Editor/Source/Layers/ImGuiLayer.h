#pragma once
#include "Fussion/Core/Layer.h"
#include "Fussion/RHI/CommandBuffer.h"

class ImGuiLayer : public Fussion::Layer {
public:
    ImGuiLayer();

    static ImGuiLayer* This() { return s_Instance; }

    /**
     * Init is used to set up ImGui and register some image callbacks. These callbacks are required to associate
     * created images with the ImGui backend and make them available for displaying. This means that any
     * image to be loaded and later displayed through ImGui MUST happen after this function.
     */
    void Init();
    virtual void OnStart() override;
    virtual void OnUpdate(f32) override;

    void Begin();
    void End(Ref<Fussion::RHI::CommandBuffer> const&);

    /**
     * This function will load various different fonts into ImGui. It must only be called AFTER Init.
     */
    void LoadFonts();

    // @Todo The VkDescriptorSet could be changed to void*.
    std::map<u64, void*> ImageToVkSet;

private:
    static ImGuiLayer* s_Instance;
    std::mutex m_RegistrationMutex;
};

void* ToImGuiTexture(Ref<Fussion::RHI::Image> const& image);

void* ToImGuiTexture(Ref<Fussion::RHI::ImageView> const& view);

#define IMGUI_IMAGE(image) ToImGuiTexture(image)

