#include "SceneWindow.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

void SceneWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Scene Entities")) {
        m_IsFocused = ImGui::IsWindowFocused();
    }
    ImGui::End();
}