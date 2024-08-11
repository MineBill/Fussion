#pragma once
#include "AssetWindow.h"

class Texture2DWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(Texture2DWindow)

    virtual void OnDraw(f32 delta) override;
    virtual void OnSave() override;
};
