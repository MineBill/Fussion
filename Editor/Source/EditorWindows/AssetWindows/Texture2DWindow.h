#pragma once
#include "AssetWindow.h"
#include "Fussion/Math/Vector2.h"

class Texture2DWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(Texture2DWindow)

    virtual void OnDraw(f32 delta) override;
    virtual void OnSave() override;

private:
    Vector2 m_uv0{ 0, 0 }, m_uv1{ 1, 1 };
};
