#pragma once
#include "AssetWindow.h"
#include "Fussion/Math/Vector2.h"

class Texture2DWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(Texture2DWindow)

    virtual void OnDraw(f32 delta) override;
    virtual void OnSave() override;

private:
    Vector2 m_UvO{ 0, 0 }, m_Uv1{ 1, 1 };
    f32 m_Zoom{ 1.0f };

    f32 m_ZoomSpeed{ 0.01f };
};
