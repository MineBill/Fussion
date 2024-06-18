#include "ViewportWindow.h"
#include "Layers/Editor.h"
#include "EditorApplication.h"
#include "SceneRenderer.h"
#include "Layers/ImGuiLayer.h"
#include "Fussion/Assets/AssetManager.h"

#include <cmath>
#include <imgui.h>
#include <tracy/Tracy.hpp>

bool operator==(const Vector2& vec, const Vector2& rhs)
{
    return fabs(vec.x - rhs.x) <= FLT_EPSILON && fabs(vec.y - rhs.y) <= FLT_EPSILON;
}

void ViewportWindow::OnDraw()
{
    ZoneScoped;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(0, 0));
    defer (ImGui::PopStyleVar());
    if (ImGui::Begin("Viewport")) {
        m_IsFocused = ImGui::IsWindowHovered();

        if (const auto size = ImGui::GetContentRegionAvail(); m_Size != size) {
            Editor::OnViewportResized(size);
            // EditorApplication::Instance()->Resize(size);
            m_Size = size;
        }

        {
            ZoneScopedN("Get image from set");
            const auto image = Editor::Get().GetSceneRenderer().GetFrameBuffer()->GetColorAttachment(0);
            ImGui::Image(IMGUI_IMAGE(image), m_Size);
        }

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(5, 5));
        defer (ImGui::PopStyleVar());
        if (ImGui::Begin("Example: Simple overlay", nullptr, window_flags)) {
            const ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("Simple overlay\n" "(right-click to change position)");
            ImGui::Separator();
            auto pos = Editor::GetCamera().GetPosition();
            ImGui::Text("Mouse Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
        }
        ImGui::End();

        if (ImGui::BeginDragDropTarget()) {
            if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET"); payload) {
                auto handle = CAST(Fussion::AssetHandle*, payload->Data);

                auto scene = Fsn::AssetManager::GetAsset<Fussion::Scene>(*handle);
                Editor::ChangeScene(scene);
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}