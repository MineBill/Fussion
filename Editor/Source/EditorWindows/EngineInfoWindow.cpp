#include "EditorUI.h"

#include "EngineInfoWindow.h"

#include "Fussion/OS/System.h"
#include "imgui.h"

static void HelpMarker(char const* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void EngineInfoWindow::OnDraw()
{
    if (!IsVisible())
        return;

    EUI::Window("Engine Info", [&] {
        ImGui::BeginTable("awdawd", 2);
        ImGui::TableNextColumn();

        ImGui::Text("Operating System");
        ImGui::TableNextColumn();
#ifdef OS_LINUX
        ImGui::Text("Linux");
#elifdef OS_WINDOWS
        ImGui::Text("Windows");
#endif
        ImGui::TableNextColumn();

        auto const& info = Fussion::System::GetSystemInfo();

        ImGui::Text("Window System");
        ImGui::SameLine(); HelpMarker("This is the window system the Editor uses and might not reflect the actual system used by the desktop environment");
        ImGui::TableNextColumn();
        ImGui::Text("%s", magic_enum::enum_name(info.WindowingSystem).data());
        ImGui::TableNextColumn();

        ImGui::Text("Desktop Environment");
        ImGui::TableNextColumn();
        ImGui::Text("%s", magic_enum::enum_name(info.Desktop).data());
        ImGui::TableNextColumn();

        ImGui::EndTable();
    },
        { .Opened = &m_IsVisible });
}
