#include "EditorPCH.h"
#include "ScriptsInspector.h"

#include "Fussion/Scripting/ScriptingEngine.h"
#include "Fussion/Scripting/Attributes/RangeAttribute.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

#include "Layers/Editor.h"

void ScriptsInspector::on_draw()
{
    if (!is_visible())
        return;

    if (ImGui::Begin("Scripts Inspector", &m_is_visible)) {
        m_is_focused = ImGui::IsWindowFocused();

        auto assembly = Fussion::ScriptingEngine::inst().get_assembly("Game");

        ImGui::TextUnformatted("Inspecting Game assembly");
        ImGui::BeginChild("Types", Vector2(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        if (assembly) {
            for (auto& [name, klass] : assembly->get_all_classes()) {
                if (ImGui::Selectable(name.c_str())) {
                    m_selected_class = &klass;
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Information", Vector2{}, ImGuiChildFlags_Border);
        if (m_selected_class) {
            ImGui::PushFont(EditorStyle::get_style().fonts[EditorFont::BoldSmall]);
            ImGui::TextUnformatted(m_selected_class->name().c_str());
            ImGui::PopFont();

            ImGui::Separator();

            for (auto& [name, prop] : m_selected_class->get_properties()) {
                auto range = Fussion::ScriptingEngine::get_attribute<Fussion::Scripting::RangeAttribute>(prop.uuid);
                if (range != nullptr) {
                    ImGui::Text("[%s]", range->to_string().c_str());
                }
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextUnformatted(magic_enum::enum_name(prop.type_id).data());

            }

            for (auto const& name : m_selected_class->get_methods() | std::views::keys) {
                if (ImGui::Button(name.c_str())) {
                    auto instance = m_selected_class->create_instance();
                    LOG_DEBUGF("Calling {}", name);
                    instance.call_method(name);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
