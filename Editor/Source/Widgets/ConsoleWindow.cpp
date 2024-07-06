#include "ConsoleWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <misc/cpp/imgui_stdlib.h>

#include "ImGuiHelpers.h"
#include "Assets/Importers/TextureImporter.h"
#include "Fussion/Log/Log.h"
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"

void ConsoleWindow::OnStart()
{
    m_ErrorIcon = TextureImporter::LoadTextureFromFile("Assets/Icons/ErrorIcon.png");
    m_InfoIcon = TextureImporter::LoadTextureFromFile("Assets/Icons/InfoIcon.png");
    m_WarningIcon = TextureImporter::LoadTextureFromFile("Assets/Icons/WarningIcon.png");
}

void ConsoleWindow::OnDraw()
{
    ZoneScoped;
    for (auto const& entry : Editor::Get().GetLogEntries()) {
        m_LogEntries.push_back(entry);
    }
    if (ImGui::Begin("Console")) {
        m_IsFocused = ImGui::IsWindowFocused();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(5, 5));

        ImGuiH::ImageToggleButton("##info_toggle", m_InfoIcon->GetImage(), m_InfoEnable, Vector2(15, 15)); ImGui::SameLine();
        ImGuiH::ImageToggleButton("##warn_toggle", m_WarningIcon->GetImage(), m_WarningEnabled, Vector2(15, 15)); ImGui::SameLine();
        ImGuiH::ImageToggleButton("##error_toggle", m_ErrorIcon->GetImage(), m_ErrorEnabled, Vector2(15, 15));

        ImGui::PopStyleVar();

        ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();

        ImGui::TextUnformatted("Auto Scroll"); ImGui::SameLine();
        ImGui::Checkbox("##auto_scroll", &m_AutoScroll); ImGui::SameLine();

        ImGui::TextUnformatted("Search:");
        ImGui::SameLine();

        static std::string search_term;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 50);
        ImGui::InputTextWithHint("##search_input_label", "Enter search term..", &search_term);

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            m_LogEntries.clear();
        }

        ImGui::Separator();
        if (ImGui::BeginChild("##console_content")) {
            for (const auto& entry : m_LogEntries) {
                if (entry.Message.find(search_term) != std::string::npos) {
                    Vector4 text_color{};
                    switch (entry.Level) {
                    using enum Fsn::LogLevel;
                    case Debug:
                    case Info:
                        if (!m_InfoEnable) continue;
                        text_color = Vector4(1, 1, 1, 1);
                        break;
                    case Warning:
                        if (!m_WarningEnabled) continue;
                        text_color = Vector4(1, 1, 0, 1);
                        break;
                    case Error:
                        if (!m_ErrorEnabled) continue;
                    case Fatal:
                        text_color = Vector4(1, 0, 0, 1);
                        break;
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                    ImGui::TextUnformatted(std::format("[{}]: {}", magic_enum::enum_name(entry.Level), entry.Message).c_str());
                    ImGui::PopStyleColor();

                    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                        ImGui::SetScrollHereY(1.0);
                    }
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}