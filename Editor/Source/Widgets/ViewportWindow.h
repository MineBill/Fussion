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

    void OnDraw() override;
    void OnEvent(Fussion::Event& event) override;

    Vector2 GetSize() const { return m_Size; }

private:
    Vector2 m_Size;
    Vector2 m_ContentOriginScreen;

    GizmoMode m_GizmoMode{GizmoMode::Translation};
    GizmoSpace m_GizmoSpace{GizmoSpace::Local};
};
