#pragma once
#include "AssetWindow.h"

class MaterialWindow : public AssetWindow {
public:
    ASSET_WINDOW_DEFAULT(MaterialWindow)

    virtual void on_draw(f32 delta) override;
    virtual void on_save() override;
};
