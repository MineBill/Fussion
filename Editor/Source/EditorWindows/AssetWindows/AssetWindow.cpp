#include "EditorPCH.h"
#include "AssetWindow.h"

#include "EditorUI.h"
#include "imgui.h"

#include <fmt/format.h>

using namespace Fussion;

void AssetWindow::draw(f32 delta)
{
    ImGui::PushID(CAST(s32, m_asset_handle));
    defer(ImGui::PopID());
    auto window_name = fmt::format("Asset Window##{}", m_asset_handle);
    EUI::window(window_name, [&] {
        draw_menu_bar();

        ImGui::BeginChild("inner_child", {}, ImGuiChildFlags_Border);
        defer(ImGui::EndChild());

        on_draw(delta);

    }, { .style = WindowStyleAssetPreview, .opened = &m_opened, .flags = ImGuiWindowFlags_MenuBar, .use_child = false });
}

void AssetWindow::draw_menu_bar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Asset")) {
            if (ImGui::MenuItem("Save...", "Ctrl+S")) {
                on_save();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
