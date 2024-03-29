#pragma once
#include <Fussion/Rendering/Framebuffer.h>
#include <Fussion/Scene/Components.h>
#include <Fussion/Scene/Scene.h>

namespace Editor
{
    using namespace Fussion;

    class ViewportPanel final
    {
        glm::vec2 m_size{};
        /// Top-left position of the panel in window-space. Does not include the header height
        glm::vec2 m_position{};
        bool m_is_focused{false};
        Ref<Framebuffer> m_frame_buffer{};

    public:
        ViewportPanel();
        void on_draw(Optional<Entity> selected, Entity camera_entity, Scene &scene, f32 delta);
        bool on_event(Event &e);

        mustuse const glm::vec2 &size() const { return m_size; }
        mustuse const glm::vec2 &position() const { return m_position; }
        mustuse Framebuffer *framebuffer() const { return m_frame_buffer.get(); }
        mustuse bool is_focused() const { return m_is_focused; }
    };
} // namespace Editor
