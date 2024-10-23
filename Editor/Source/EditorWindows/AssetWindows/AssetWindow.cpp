#include "EditorPCH.h"
#include "AssetWindow.h"

#include "EditorUI.h"
#include "imgui.h"

#include <fmt/format.h>

using namespace Fussion;

void AssetWindow::Draw(f32 delta)
{
    ImGui::PushID(CAST(s32, m_AssetHandle));
    defer(ImGui::PopID());
    auto window_name = fmt::format("Asset Window##{}", m_AssetHandle);
    EUI::Window(window_name, [&] {
        DrawMenuBar();

        EUI::ImGuiStyleBuilder()
            .With(ImGuiCol_ChildBg, Color::Transparent)
            .Do([&] {
                ImGui::BeginChild("inner_child", {}, ImGuiChildFlags_Border);
                defer(ImGui::EndChild());

                OnDraw(delta);
            });
    },
        { .Style = WindowStyleAssetPreview, .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar, .UseChild = false });
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
