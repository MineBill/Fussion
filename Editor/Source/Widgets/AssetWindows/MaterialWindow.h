#pragma once
#include "AssetWindow.h"

class MaterialWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(MaterialWindow)

    void OnDraw(f32 delta) override;
    void OnSave() override;
};
