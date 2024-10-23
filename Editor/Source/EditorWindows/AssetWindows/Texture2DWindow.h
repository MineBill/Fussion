#pragma once
#include "AssetWindow.h"
#include "Fussion/Math/Vector2.h"

class Texture2DWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(Texture2DWindow)

    virtual void OnDraw(f32 delta) override;
    virtual void OnSave() override;

private:
    Vector2 m_PanPosition { 0.5f, 0.5f };
    Vector2 m_Scale { 1.0f, 1.0f };
    f32 m_ZoomRate { 1.2f };
    bool m_IsDragging {};
};
