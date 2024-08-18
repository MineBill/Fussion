#include "epch.h"
#include "ScriptsInspector.h"

#include "Fussion/Scripting/ScriptingEngine.h"
#include "Fussion/Scripting/Attributes/RangeAttribute.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

#include "Layers/Editor.h"

void ScriptsInspector::OnDraw()
{
    if (!IsVisible())
        return;

    if (ImGui::Begin("Scripts Inspector", &m_IsVisible)) {
        m_IsFocused = ImGui::IsWindowFocused();

        auto assembly = Fussion::ScriptingEngine::Get().GetAssembly("Game");

        ImGui::TextUnformatted("Inspecting Game assembly");
        ImGui::BeginChild("Types", Vector2(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        if (assembly) {
            for (auto& [name, klass] : assembly->GetAllClasses()) {
                if (ImGui::Selectable(name.c_str())) {
                    m_SelectedClass = &klass;
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Information", Vector2{}, ImGuiChildFlags_Border);
        if (m_SelectedClass) {
            ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::BoldSmall]);
            ImGui::TextUnformatted(m_SelectedClass->GetName().c_str());
            ImGui::PopFont();

            ImGui::Separator();

            for (auto& [name, prop] : m_SelectedClass->GetProperties()) {
                auto range = Fussion::ScriptingEngine::GetAttribute<Fussion::Scripting::RangeAttribute>(prop.Uuid);
                if (range != nullptr) {
                    ImGui::Text("[%s]", range->ToString().c_str());
                }
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextUnformatted(magic_enum::enum_name(prop.TypeId).data());

            }

            for (auto const& name : m_SelectedClass->GetMethods() | std::views::keys) {
                if (ImGui::Button(name.c_str())) {
                    auto instance = m_SelectedClass->CreateInstance();
                    LOG_DEBUGF("Calling {}", name);
                    instance.CallMethod(name);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
