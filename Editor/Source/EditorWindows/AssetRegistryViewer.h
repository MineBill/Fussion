﻿#pragma once
#include "EditorWindow.h"

class AssetRegistryViewer final : public EditorWindow {
public:
    EDITOR_WINDOW(AssetRegistryViewer)

    virtual void OnDraw() override;
};