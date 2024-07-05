#include "ViewportWindow.h"
#include "Layers/Editor.h"
#include "EditorApplication.h"
#include "SceneRenderer.h"
#include "Layers/ImGuiLayer.h"
#include "Fussion/Assets/AssetManager.h"

#include <cmath>
#include <imgui.h>
#include <tracy/Tracy.hpp>

using namespace Fussion;

bool operator==(const Vector2& vec, const Vector2& rhs)
{
    return fabs(vec.X - rhs.X) <= FLT_EPSILON && fabs(vec.Y - rhs.Y) <= FLT_EPSILON;
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
            const auto image = Editor::Get().GetSceneRenderer().GetFrameBuffer()->GetColorAttachment(1);
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
            ImGui::Text("Mouse Position: {}", pos);
        }
        ImGui::End();

        if (ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::GetDragDropPayload();
            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                auto handle = CAST(AssetHandle*, payload->Data);

                auto metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(*handle);
                if (metadata.Type == AssetType::Scene) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        auto scene = AssetManager::GetAsset<Scene>(*handle);
                        Editor::ChangeScene(scene);
                    }
                } else if (metadata.Type == AssetType::Texture2D) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        Editor::Get().TextureRef = AssetManager::GetAsset<Texture2D>(*handle);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}
