#include "ScriptsInspector.h"

#include "Engin5/Scripting/ScriptingEngine.h"
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

#include "Layers/Editor.h"

void ScriptsInspector::OnDraw()
{
    if (!IsVisible()) return;

    if (ImGui::Begin("Scripts Inspector", &m_IsVisible))
    {
        m_IsFocused = ImGui::IsWindowFocused();

        auto assembly = Engin5::ScriptingEngine::Get().GetAssembly("Game");

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
            ImGui::PushFont(Editor::Get().GetStyle().Fonts.BoldSmall);
            ImGui::TextUnformatted(m_SelectedClass->GetName().c_str());
            ImGui::PopFont();

            ImGui::Separator();

            for (auto& [name, method] : m_SelectedClass->GetProperties()) {
                ImGui::TextUnformatted(name.c_str());
                ImGui::TextUnformatted(magic_enum::enum_name(method.TypeId).data());
            }

            // for (auto& [name, method] : m_SelectedClass->GetMethods()) {
            //     if (ImGui::Button(std::format("Invoke '{}'", name).c_str())) {
            //     }
            // }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}