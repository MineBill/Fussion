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
    }
    ImGui::End();
}