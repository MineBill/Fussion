#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Assets/Asset.h"

class AssetWindow {
public:
    explicit AssetWindow(Fussion::AssetHandle handle): m_asset_handle(handle) {}
    virtual ~AssetWindow() = default;

    void draw(f32 delta);

    /**
     * Called every frame.
     * @param delta Delta time.
     */
    virtual void on_draw(f32 delta) = 0;

    /**
     * Called when the user/editor wants to save the asset being modified/previewed by this asset window.
     */
    virtual void on_save() = 0;

    /**
     * @return True if the asset window is open.
     */
    bool is_open() const { return m_opened; }

protected:
    /**
     * Draws the menu bar for this asset.
     * @attention Must be called from inside the window code.
     */
    void draw_menu_bar();

    bool m_opened{ true };
    Fussion::AssetHandle m_asset_handle{ 0 };
};

#define ASSET_WINDOW_DEFAULT(klass) \
    explicit klass(Fussion::AssetHandle handle): AssetWindow(handle) {}
