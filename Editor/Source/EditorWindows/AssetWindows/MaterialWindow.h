#pragma once
#include "AssetWindow.h"

class MaterialWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(MaterialWindow)

    virtual void OnDraw(f32 delta) override;
    virtual void OnSave() override;
};
