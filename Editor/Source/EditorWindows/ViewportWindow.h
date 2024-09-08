#pragma once
#include "EditorWindow.h"
#include "Fussion/Math/Vector2.h"

class ViewportWindow final : public EditorWindow {
public:
    enum class GizmoMode {
        Translation,
        Rotation,
        Scale,
    };

    enum class GizmoSpace {
        Local,
        World,
    };

    EDITOR_WINDOW(ViewportWindow)

    void render_stats() const;
    virtual void on_draw() override;
    virtual void on_event(Fussion::Event& event) override;

    Vector2 size() const { return m_size; }

private:
    Vector2 m_size;
    Vector2 m_content_origin_screen;

    GizmoMode m_gizmo_mode{ GizmoMode::Translation };
    GizmoSpace m_gizmo_space{ GizmoSpace::Local };
};
