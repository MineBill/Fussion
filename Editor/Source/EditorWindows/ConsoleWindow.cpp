#include "EditorPCH.h"
#include "ConsoleWindow.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <misc/cpp/imgui_stdlib.h>

#include "ImGuiHelpers.h"
#include <Fussion/Util/TextureImporter.h>
#include "Fussion/Log/Log.h"
#include "Layers/Editor.h"

void ConsoleWindow::on_start() {}

void ConsoleWindow::on_draw()
{
    ZoneScoped;
    // auto const& entries = Editor::inst().log_entries();
    // std::vector<Fsn::LogEntry> {};
    // std::ranges::copy(entries, std::back_inserter(m_log_entries));

    if (ImGui::Begin("Console")) {
        m_is_focused = ImGui::IsWindowFocused();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(5, 5));

        auto& style = EditorStyle::get_style();
        ImGuiH::ImageToggleButton("##info_toggle", style.editor_icons[EditorIcon::Info]->image().view, m_info_enable, Vector2(15, 15));
        ImGui::SameLine();
        ImGuiH::ImageToggleButton("##warn_toggle", style.editor_icons[EditorIcon::Warning]->image().view, m_warning_enabled, Vector2(15, 15));
        ImGui::SameLine();
        ImGuiH::ImageToggleButton("##error_toggle", style.editor_icons[EditorIcon::Error]->image().view, m_error_enabled, Vector2(15, 15));

        ImGui::PopStyleVar();

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        ImGui::TextUnformatted("Auto Scroll");
        ImGui::SameLine();
        ImGui::Checkbox("##auto_scroll", &m_auto_scroll);
        ImGui::SameLine();

        ImGui::TextUnformatted("Search:");
        ImGui::SameLine();

        static std::string search_term;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 50);
        ImGui::InputTextWithHint("##search_input_label", "Enter search term..", &search_term);

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            m_log_entries.clear();
        }

        ImGui::Separator();
        if (ImGui::BeginChild("##console_content")) {
            for (const auto& entry : m_log_entries) {
                if (entry.message.find(search_term) != std::string::npos) {
                    Vector4 text_color{};
                    switch (entry.level) {
                        using enum Fsn::LogLevel;
                    case Debug:
                    case Info:
                        if (!m_info_enable)
                            continue;
                        text_color = Vector4(1, 1, 1, 1);
                        break;
                    case Warning:
                        if (!m_warning_enabled)
                            continue;
                        text_color = Vector4(1, 1, 0, 1);
                        break;
                    case Error:
                        if (!m_error_enabled)
                            continue;
                    case Fatal:
                        text_color = Vector4(1, 0, 0, 1);
                        break;
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                    ImGui::TextUnformatted(std::format("[{}]: {}", magic_enum::enum_name(entry.level), entry.message).c_str());
                    ImGui::PopStyleColor();

                    if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                        ImGui::SetScrollHereY(1.0);
                    }
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
