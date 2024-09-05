#include "EditorPCH.h"
#include "ImGuiHelpers.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Math/Rect.h"

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Layers/ImGuiLayer.h"

static ImVector<ImRect> s_GroupPanelLabelStack;

void ImGuiHelpers::BeginProperty(const char* label)
{
    constexpr auto table_flags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoSavedSettings |
        ImGuiTableFlags_SizingStretchSame;
    ImGui::BeginTable(label, 2, table_flags);
}

void ImGuiHelpers::EndProperty()
{
    ImGui::EndTable();
}

void ImGuiHelpers::BeginGroupPanel(const char* name, const ImVec2& size, ImFont* font)
{
    ImGui::PushFont(font);
    defer(ImGui::PopFont());

    ImGui::BeginGroup();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(2.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vector2(2.0f, 2.0f));

    auto frameHeight = ImGui::GetFrameHeight();
    ImGui::BeginGroup();

    ImVec2 effective_size;
    if (size.x < 0.0f)
        effective_size.x = ImGui::GetContentRegionAvail().x;
    else
        effective_size.x = size.x;
    ImGui::Dummy(ImVec2(effective_size.x, 0.0f));

    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextUnformatted(name);
    auto labelMin = ImGui::GetItemRectMin();
    auto labelMax = ImGui::GetItemRectMax();
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
    ImGui::BeginGroup();

    // ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x -= frameHeight;

    auto itemWidth = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

    s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
}

void ImGuiHelpers::EndGroupPanel()
{
    ImGui::PopItemWidth();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));

    auto frameHeight = ImGui::GetFrameHeight();

    ImGui::EndGroup();

    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(Vector2(frameHeight * 0.5f, 0.0f));
    ImGui::Dummy(Vector2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

    ImGui::EndGroup();

    Vector2 item_min = ImGui::GetItemRectMin();
    Vector2 item_max = ImGui::GetItemRectMax();

    auto label_rect = s_GroupPanelLabelStack.back();
    s_GroupPanelLabelStack.pop_back();

    auto half_frame = Vector2(frameHeight * 0.25f, frameHeight) * 0.5f;
    auto frame_rect = ImRect(item_min + half_frame, item_max - Vector2(half_frame.X, 0.0f));
    label_rect.Min.x -= itemSpacing.x;
    label_rect.Max.x += itemSpacing.x;
    for (int i = 0; i < 4; ++i) {
        switch (i) {
        // left half-plane
        case 0:
            ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(label_rect.Min.x, FLT_MAX), true);
            break;
        // right half-plane
        case 1:
            ImGui::PushClipRect(ImVec2(label_rect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true);
            break;
        // top
        case 2:
            ImGui::PushClipRect(ImVec2(label_rect.Min.x, -FLT_MAX), ImVec2(label_rect.Max.x, label_rect.Min.y), true);
            break;
        // bottom
        case 3:
            ImGui::PushClipRect(ImVec2(label_rect.Min.x, label_rect.Max.y), ImVec2(label_rect.Max.x, FLT_MAX), true);
            break;
        }

        ImGui::GetWindowDrawList()->AddRect(frame_rect.Min, frame_rect.Max,
            ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)), half_frame.X);

        ImGui::PopClipRect();
    }

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x += frameHeight;

    ImGui::Dummy(ImVec2(0.0f, 0.0f));

    ImGui::EndGroup();
}

bool ImGuiHelpers::DragVec3(const char* id, Vector3* value, f32 speed, f32 min, f32 max, const char* format, ImFont* font, ImFont* font2)
{
    bool modified{ false };
    ImGui::PushID(id);
    constexpr auto x_color = ImVec4(0.92f, 0.24f, 0.27f, 1.0);
    constexpr auto x_color_hover = ImVec4(0.76f, 0.20f, 0.22f, 1.0);
    constexpr auto y_color = ImVec4(0.20f, 0.67f, 0.32f, 1.0);
    constexpr auto y_color_hover = ImVec4(0.15f, 0.52f, 0.25f, 1.0);
    constexpr auto z_color = ImVec4(0.18f, 0.49f, 0.74f, 1.0);
    constexpr auto z_color_hover = ImVec4(0.14f, 0.39f, 0.60f, 1.0);
    constexpr s32 space_count = 5;
    constexpr f32 item_count = 3;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
    auto spacing = ImGui::GetStyle().ItemSpacing.x;
    auto width =
        (ImGui::GetContentRegionAvail().x - spacing * space_count - ImGui::CalcTextSize("X").x * 3) / item_count;

    ImGui::AlignTextToFramePadding();

    auto DrawLabel = [font](const char* label, ImVec4 color) {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::PushFont(font);
        ImGui::Text(label);
        ImGui::PopFont();
        ImGui::PopStyleColor();
    };

    DrawLabel("X", x_color);
    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, x_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, x_color_hover);
    ImGui::PushFont(font2);
    modified |= ImGui::DragFloat("##x", &value->X, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    DrawLabel("Y", y_color);

    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, y_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, y_color_hover);
    ImGui::PushFont(font2);
    modified |= ImGui::DragFloat("##y", &value->Y, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    DrawLabel("Z", z_color);

    ImGui::SameLine();
    ImGui::PushItemWidth(width);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, z_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, z_color_hover);
    ImGui::PushFont(font2);
    modified |= ImGui::DragFloat("##z", &value->Z, speed, min, max, format);
    ImGui::PopFont();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::PopStyleVar();
    ImGui::PopID();
    return modified;
}

bool ImGuiHelpers::ButtonCenteredOnLine(const char* label, float alignment)
{
    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Button(label);
}

void ImGuiHelpers::RenderSimpleRect(ImDrawList* draw_list, Vector2 const& position, Vector2 const& size, u32 color,
    f32 width)
{
    ImGui::RenderRectFilledWithHole(
        draw_list, ImRect(position.X, position.Y, position.X + size.X, position.Y + size.Y),
        ImRect(position.X + width, position.Y + width, position.X + size.X - width, position.Y + size.Y - width), color,
        0.0f);
}

void ImGuiHelpers::InputText(const char* label, std::string& value, ImGuiInputTextFlags flags)
{
    BeginProperty(label);
    defer(EndProperty());

    if (flags & ImGuiInputTextFlags_ReadOnly)
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushID(label);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##input", &value, flags);
    ImGui::PopID();
}

bool ImGuiHelpers::ImageToggleButton(const char* id, Fussion::GPU::TextureView const& texture, bool& toggled, Vector2 const& size)
{
    auto button = CAST(Vector4, ImGui::GetStyleColorVec4(ImGuiCol_Button)) * 0.8f;

    if (toggled) {
        ImGui::PushStyleColor(ImGuiCol_Button, button);
    }
    const auto pressed = ImGui::ImageButton(id, texture, size);
    if (toggled) {
        ImGui::PopStyleColor();
    }

    if (pressed) {
        toggled = !toggled;
    }
    return pressed;
}

bool ImGuiHelpers::TreeNode(std::string_view label, Fussion::GPU::TextureView const& view, ImGuiTreeNodeFlags flags)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;

    ImGuiID id = window->GetID(label.data());
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
    bool opened = ImGui::TreeNodeBehaviorIsOpen(id, flags);
    // bool hovered, held;
    // bool selected = flags & ImGuiTreeNodeFlags_Selected;

    float button_sz = g.FontSize + g.Style.FramePadding.y * 2;
    auto arrow_rect = Fussion::Rect::FromStartEnd(pos, Vector2(pos.x + button_sz, bb.Max.y));
    //
    // if (flags & ImGuiTreeNodeFlags_OpenOnArrow) {
    //     if (ImGui::ButtonBehavior(ImRect(pos, ImVec2(pos.x + button_sz, bb.Max.y)), id, &hovered, &held, ImGuiButtonFlags_PressedOnClick))
    //         window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
    // } else {
    //     if (ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick))
    //         window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
    // }
    //
    // if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) {
    //     if (ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnDoubleClick))
    //         window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
    // }
    //

    auto text_pos = Vector2(pos.x + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y);

    const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
    ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
    if ((flags & ImGuiTreeNodeFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap))
        button_flags |= ImGuiButtonFlags_AllowOverlap;
    if (!is_leaf)
        button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

    auto& style = ImGui::GetStyle();
    (void)style;

    // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
    // Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
    // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
    // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
    // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
    // - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
    // - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
    // It is rather standard that arrow click react on Down rather than Up.
    // We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
    auto is_mouse_x_over_arrow = arrow_rect.Contains(g.IO.MousePos);
    if (is_mouse_x_over_arrow)
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    else
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease;

    bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
    const bool was_selected = selected;

    bool is_open = ImGui::TreeNodeUpdateNextOpen(id, flags);

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);
    bool toggled = false;
    if (!is_leaf) {
        if (pressed && g.DragDropHoldJustPressedId != id) {
            if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
                toggled = true;
            if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
                toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
            if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseClickedCount[0] == 2)
                toggled = true;
        } else if (pressed && g.DragDropHoldJustPressedId == id) {
            IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
            if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
                toggled = true;
        }

        if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && is_open) {
            toggled = true;
            ImGui::NavClearPreferredPosForAxis(ImGuiAxis_X);
            ImGui::NavMoveRequestCancel();
        }
        if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
        {
            toggled = true;
            ImGui::NavClearPreferredPosForAxis(ImGuiAxis_X);
            ImGui::NavMoveRequestCancel();
        }

        if (toggled) {
            is_open = !is_open;
            window->DC.StateStorage->SetInt(id, is_open);
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
        }
    }

    // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    if (hovered || selected) {
        ImU32 const bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_col, false);
    }

    if (!(flags & ImGuiTreeNodeFlags_Leaf)) {
        ImGui::RenderArrow(window->DrawList, text_pos, 0xFFFFFFFF, opened ? ImGuiDir_Down : ImGuiDir_Right);
    }
    // Icon, text
    auto button_pos = Vector2(pos) + Vector2(button_sz, 0);
    window->DrawList->AddImage(view, button_pos, button_pos + button_sz);
    ImGui::RenderText(text_pos + Vector2(button_sz * 2, 0), label.data());

    ImGui::ItemSize(bb, g.Style.FramePadding.y);
    ImGui::ItemAdd(bb, id);

    if (opened)
        ImGui::TreePush(label.data());
    return opened;
}
