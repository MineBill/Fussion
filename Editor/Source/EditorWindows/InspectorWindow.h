#pragma once
#include "AssetPicker.h"
#include "EditorWindow.h"

#include <Fussion/Scene/Entity.h>

/// The Inspector window displays the properties of the currently selected entity
/// or entities. It's also used to display the member of the components that are
/// attached to the given entity.
class InspectorWindow final : public EditorWindow {
public:
    EDITOR_WINDOW(InspectorWindow)

    virtual void OnStart() override;
    virtual void OnDraw() override;

private:
    /// Draws a component in the inspector panel.
    /// @return true if the component was modified, false otherwise.
    bool DrawComponent(Fsn::Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr);

    bool DrawProperty(meta_hpp::uvalue prop_value, meta_hpp::member const& member, meta_hpp::uvalue& ptr);

    /// Draws an entity in the inspector panel.
    /// @return true if the entity was modified, false otherwise.
    bool DrawEntity(Fussion::Entity& e);
};
