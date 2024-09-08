#pragma once
#include "AssetWindow.h"
#include "Fussion/Math/Vector2.h"

class Texture2DWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(Texture2DWindow)

    virtual void on_draw(f32 delta) override;
    virtual void on_save() override;

private:
    Vector2 m_uv0{ 0, 0 }, m_uv1{ 1, 1 };
    f32 m_zoom{ 1.0f };

    f32 m_zoom_speed{ 0.01f };
};
