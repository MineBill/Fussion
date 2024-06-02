#include "InspectorWindow.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

void InspectorWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Inspector")) {
        m_IsFocused = ImGui::IsWindowFocused();
        ImGui::Text("Inspector window");
    }
    ImGui::End();
}