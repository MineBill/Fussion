#pragma once
#include "EditorWindow.h"
#include "ImGuiHelpers.h"
#include "Fussion/Scene/Entity.h"

/// The Inspector window displays the properties of the currently selected entity
/// or entities. It's also used to display the member of the components that are
/// attached to the given entity.
class InspectorWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(InspectorWindow)

    void OnStart() override;
    void OnDraw() override;

private:
    /// Draws a component in the inspector panel.
    /// @return true if the component was modified, false otherwise.
    bool DrawComponent(Fsn::Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr);

    /// Draws an entity in the inspector panel.
    /// @return true if the entity was modified, false otherwise.
    bool DrawEntity(Fussion::Entity& e);

    template<typename F>
    void DoProperty(std::string const& label, F&& f)
    {
        ImGuiH::BeginProperty(label.c_str());
        ImGui::TableNextColumn();

        ImGui::TextUnformatted(label.c_str());
        ImGui::TableNextColumn();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        f();

        ImGuiH::EndProperty();
    }
};
