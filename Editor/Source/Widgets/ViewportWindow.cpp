#include "ViewportWindow.h"
#include "Layers/EditorLayer.h"
#include "EditorApplication.h"
#include "SceneRenderer.h"
#include "Layers/ImGuiLayer.h"

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
            EditorApplication::Instance()->Resize(size);
            m_Size = size;
        }

        {
            ZoneScopedN("Get image from set");
            const auto image = EditorApplication::Instance()->GetSceneRenderer().GetFrameBuffer()->GetColorAttachment(0);
            const auto set = ImGuiLayer::This()->ImageToVkSet[transmute(u64, image->GetRenderHandle<VkImage>())];
            ImGui::Image(set, m_Size);
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
            auto pos = EditorApplication::Instance()->GetCamera().GetPosition();
            ImGui::Text("Mouse Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
        }
        ImGui::End();
    }
    ImGui::End();
}