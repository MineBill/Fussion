#pragma once
#include "EditorWindow.h"
#include "AssetPicker.h"

#include <Fussion/Scene/Entity.h>

/// The Inspector window displays the properties of the currently selected entity
/// or entities. It's also used to display the member of the components that are
/// attached to the given entity.
class InspectorWindow final : public EditorWindow {
public:
    EDITOR_WINDOW(InspectorWindow)

    virtual void on_start() override;
    virtual void on_draw() override;

private:
    /// Draws a component in the inspector panel.
    /// @return true if the component was modified, false otherwise.
    bool draw_component(Fsn::Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr);

    bool draw_property(meta_hpp::uvalue prop_value, meta_hpp::member const& member, meta_hpp::uvalue& ptr);

    /// Draws an entity in the inspector panel.
    /// @return true if the entity was modified, false otherwise.
    bool draw_entity(Fussion::Entity& e);
};
