#include "EditorPCH.h"
#include "AssetWindow.h"

#include "EditorUI.h"
#include "imgui.h"

#include <fmt/format.h>

using namespace Fussion;

void AssetWindow::draw(f32 delta)
{
    ImGui::PushID(CAST(s32, m_AssetHandle));
    defer(ImGui::PopID());
    auto window_name = fmt::format("Asset Window##{}", m_AssetHandle);
    EUI::Window(window_name, [&] {
        DrawMenuBar();

        ImGui::BeginChild("inner_child", {}, ImGuiChildFlags_Border);
        defer(ImGui::EndChild());

        OnDraw(delta);

    }, { .style = WindowStyleAssetPreview, .opened = &m_Opened, .flags = ImGuiWindowFlags_MenuBar, .use_child = false });
}

void AssetWindow::DrawMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Asset")) {
            if (ImGui::MenuItem("Save...", "Ctrl+S")) {
                OnSave();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
