#pragma once
#include <imgui.h>

namespace Fussion
{
    class Image;
}

namespace ImGuiHelpers
{
    void BeginProperty(const char* label);
    void EndProperty();

    void BeginGroupPanel(const char *name, const ImVec2 &size = ImVec2(0, 0), ImFont* font = nullptr);
    void EndGroupPanel();
    void DragVec3(const char *id, Vector3 *value, f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f,
                  const char *format = "%.2f", ImFont* font = nullptr, ImFont* font2 = nullptr);
    bool ButtonCenteredOnLine(const char *label, float alignment = 0.5f);
    void RenderSimpleRect(ImDrawList *draw_list, Vector2 const &position, Vector2 const &size,
                          u32 color = 0xff0000ff, f32 width = 1.0f);
    void RenderLine(ImDrawList *draw_list, const Vector2 &start_position, const Vector2 &end_position,
                    u32 color = 0xff0000ff, f32 width = 1.0f);

    void InputText(const char* label, std::string &value, ImGuiInputTextFlags flags = 0);

    bool ImageToggleButton(const char* id, Ref<Fussion::Image> const& image, bool& toggled, Vector2 size);
} // namespace ImGuiHelpers

namespace ImGuiH = ImGuiHelpers;