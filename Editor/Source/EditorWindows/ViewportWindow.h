#pragma once
#include "EditorWindow.h"
#include "Fussion/Math/Vector2.h"

enum TextureViewMode {
    TEXTURE_SCENE,
    TEXTURE_GBUFFER_POSITION,
    TEXTURE_GBUFFER_NORMAL,
    TEXTURE_SSAO,
};

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

    void RenderStats() const;
    virtual void OnDraw() override;
    virtual void OnEvent(Fussion::Event& event) override;

    Vector2 Size() const { return m_Size; }

private:
    Vector2 m_Size;
    Vector2 m_ContentOriginScreen;

    TextureViewMode m_TextureViewMode { TEXTURE_SCENE };
    GizmoMode m_GizmoMode { GizmoMode::Translation };
    GizmoSpace m_GizmoSpace { GizmoSpace::Local };
};
