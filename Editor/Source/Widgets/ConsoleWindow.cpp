#include "ConsoleWindow.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

void ConsoleWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Console")) {
        m_IsFocused = ImGui::IsWindowFocused();
    }
    ImGui::End();
}