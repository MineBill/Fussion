#pragma once
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector4.h"
#include <imgui.h>

namespace Fussion::GPU {
    struct TextureView;
}

namespace Fussion::RHI {
    class Image;
}

namespace ImGuiHelpers {
    void BeginProperty(char const* label);
    void EndProperty();

    void BeginGroupPanel(char const* name, ImVec2 const& size = ImVec2(0, 0), ImFont* font = nullptr);

    void EndGroupPanel();

    bool DragVec3(
        char const* id,
        Vector3* value,
        f32 speed = 1.0f,
        f32 min = 0.0f,
        f32 max = 0.0f,
        char const* format = "%.2f",
        ImFont* font = nullptr,
        ImFont* font2 = nullptr);

    bool ButtonCenteredOnLine(char const* label, float alignment = 0.5f);

    void RenderSimpleRect(
        ImDrawList* draw_list,
        Vector2 const& position,
        Vector2 const& size,
        u32 color = 0xff0000ff,
        f32 width = 1.0f);

    void RenderLine(
        ImDrawList* draw_list,
        Vector2 const& start_position,
        Vector2 const& end_position,
        u32 color = 0xff0000ff,
        f32 width = 1.0f);

    void InputText(char const* label, std::string& value, ImGuiInputTextFlags flags = 0);

    bool ImageToggleButton(char const* id, Fussion::GPU::TextureView const& texture, bool& toggled, Vector2 const& size);

    template<typename... Args>
    void Text(fmt::format_string<Args...> fmt, Args&&... args)
    {
        ImGui::TextUnformatted(fmt::format(fmt, std::forward<Args>(args)...).c_str());
    }

    template<typename... Args>
    bool Button(fmt::format_string<Args...> fmt, Args&&... args)
    {
        return ImGui::Button(fmt::format(fmt, std::forward<Args>(args)...).c_str());
    }

    bool TreeNode(std::string_view label, Fussion::GPU::TextureView const& view, ImGuiTreeNodeFlags flags = 0);

} // namespace ImGuiHelpers

namespace ImGuiH = ImGuiHelpers;
