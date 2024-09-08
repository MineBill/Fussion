#pragma once
#include "Fussion/Core/Layer.h"
#include "Fussion/GPU/GPU.h"

class ImGuiLayer final : public Fussion::Layer {
public:
    /**
     * Init is used to set up ImGui and register some image callbacks. These callbacks are required to associate
     * created images with the ImGui backend and make them available for displaying. This means that any
     * image to be loaded and later displayed through ImGui MUST happen after this function.
     */
    void init();
    virtual void on_start() override;
    virtual void on_update(f32) override;

    void begin();

    /// Calls the necessary ImGui function to end the frame.
    /// @param encoder In the case the surface cannot return
    /// a view, the encoder should be empty to indicate that
    /// no rendering is to be performed.
    void end(Maybe<Fussion::GPU::RenderPassEncoder> encoder);

private:
    /// This function will load various different fonts into ImGui. It must only be called AFTER Init.
    void load_fonts();
    void setup_im_gui_style();

    // std::mutex m_RegistrationMutex;
};
