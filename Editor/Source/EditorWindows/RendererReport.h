#pragma once
#include "EditorWindow.h"

/// The Inspector window displays the properties of the currently selected entity
/// or entities. It's also used to display the member of the components that are
/// attached to the given entity.
class RendererReport final : public EditorWindow {
public:
    EDITOR_WINDOW(RendererReport)

    virtual void on_draw() override;
};
