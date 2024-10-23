#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Types.h"

class AssetWindow {
public:
    explicit AssetWindow(Fussion::AssetHandle handle)
        : m_AssetHandle(handle)
    { }
    virtual ~AssetWindow() = default;

    void Draw(f32 delta);

    /**
     * Called every frame.
     * @param delta Delta time.
     */
    virtual void OnDraw(f32 delta) = 0;

    /**
     * Called when the user/editor wants to save the asset being modified/previewed by this asset window.
     */
    virtual void OnSave() = 0;

    /**
     * @return True if the asset window is open.
     */
    bool IsOpen() const { return m_Opened; }

protected:
    /**
     * Draws the menu bar for this asset.
     * @attention Must be called from inside the window code.
     */
    void DrawMenuBar();

    bool m_Opened { true };
    Fussion::AssetHandle m_AssetHandle { 0 };
};

#define ASSET_WINDOW_DEFAULT(klass)             \
    explicit klass(Fussion::AssetHandle handle) \
        : AssetWindow(handle)                   \
    { }
