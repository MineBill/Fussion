#include "epch.h"
#include "AssetWindow.h"

#include "EditorUI.h"
#include "imgui.h"

using namespace Fussion;

void AssetWindow::Draw(f32 delta)
{
    ImGui::PushID(m_AssetHandle);
    defer(ImGui::PopID());
    auto window_name = std::format("Asset Window##{}", m_AssetHandle);
    EUI::Window(window_name, [&] {
        DrawMenuBar();

        ImGui::BeginChild("inner_child", {}, ImGuiChildFlags_Border);
        defer(ImGui::EndChild());

        OnDraw(delta);

    }, { .Style = WindowStyleAssetPreview, .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar, .UseChild = false });
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
