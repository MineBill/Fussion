#include "EditorPCH.h"
#include "ImGuiLayer.h"

#include "Editor.h"
#include "EditorStyle.h"

#include "Fussion/Core/Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "ImGuizmo.h"

#include "Fussion/GPU/EnumConversions.h"
#include "Fussion/Rendering/Renderer.h"

void ImGuiLayer::load_fonts()
{
    auto& io = ImGui::GetIO();
    auto& style = EditorStyle::style();
    using enum EditorFont;
    style.fonts[RegularNormal] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 15.0f);
    style.fonts[RegularBig] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 18.0f);
    style.fonts[RegularSmall] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 13.0f);
    style.fonts[RegularHuge] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Regular.ttf", 24.0f);
    style.fonts[Bold] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Bold.ttf", 15.0f);
    style.fonts[BoldSmall] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Inter-Bold.ttf", 12.0f);
    style.fonts[MonospaceRegular] = io.Fonts->AddFontFromFileTTF("Assets/Fonts/JetBrainsMono-Regular.ttf", 15.0f);

    io.FontDefault = style.fonts[RegularNormal];
}

void ImGuiLayer::initialize()
{
    ZoneScoped;
    using namespace Fussion;
    LOG_DEBUGF("Initializing ImGui layer.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();

    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
#if 0
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

    auto window = CAST(GLFWwindow*, Application::self()->window().native_handle());

    ImGui_ImplGlfw_InitForOther(window, true);

    ImGui_ImplWGPU_InitInfo info {};
    info.Device = Renderer::device().As<WGPUDevice>();
    info.RenderTargetFormat = ToWGPU(Renderer::surface().Format);
    ImGui_ImplWGPU_Init(&info);

    setup_imgui_style();
    load_fonts();
}

void ImGuiLayer::on_start() { }

void ImGuiLayer::on_update(f32 delta)
{
    (void)delta;
}

void ImGuiLayer::begin()
{
    ZoneScoped;
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplWGPU_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void ImGuiLayer::end(Maybe<Fussion::GPU::RenderPassEncoder> encoder)
{
    ZoneScoped;
    ImGui::Render();
    auto io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // auto ctx = ImGui::GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        // ImGui::SetCurrentContext(ctx);
    }

    if (encoder.has_value()) {
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), encoder->As<WGPURenderPassEncoder>());
    }
}

void ImGuiLayer::setup_imgui_style()
{
    // Fork of Photoshop style from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6f;
    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.WindowRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 4.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.FrameRounding = 4.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 13.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabMinSize = 7.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 2.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.DockingSeparatorSize = 2;
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    constexpr Color foreground = Color::from_hex(0x313131FF);
    constexpr Color background = Color::from_hex(0x212121FF);
    constexpr Color frame = Color::from_hex(0x212121FF);
    constexpr Color popup = foreground;
    constexpr Color button = Color::from_hex(0x454545FF);
    constexpr Color textColor = Color::from_hex(0xe7e7e7FF);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = foreground;
    colors[ImGuiCol_ChildBg] = background;
    colors[ImGuiCol_PopupBg] = popup;
    colors[ImGuiCol_Border] = background;
    colors[ImGuiCol_BorderShadow] = Color::Transparent;
    colors[ImGuiCol_FrameBg] = frame;
    colors[ImGuiCol_FrameBgHovered] = frame.lighten(0.1f);
    colors[ImGuiCol_FrameBgActive] = frame.lighten(0.2f);
    colors[ImGuiCol_TitleBg] = background;
    colors[ImGuiCol_TitleBgActive] = background;
    colors[ImGuiCol_TitleBgCollapsed] = background;
    colors[ImGuiCol_MenuBarBg] = background;
    colors[ImGuiCol_ScrollbarBg] = button;
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = button.lighten(0.02f);
    colors[ImGuiCol_ScrollbarGrabActive] = button.lighten(0.02f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_Button] = button;
    colors[ImGuiCol_ButtonHovered] = button.lighten(0.02f);
    colors[ImGuiCol_ButtonActive] = button.lighten(0.02f);
    colors[ImGuiCol_Header] = button;
    colors[ImGuiCol_HeaderHovered] = button.lighten(0.02f);
    colors[ImGuiCol_HeaderActive] = button.darken(0.02f);
    colors[ImGuiCol_Separator] = button;
    colors[ImGuiCol_SeparatorHovered] = button.lighten(0.02f);
    colors[ImGuiCol_SeparatorActive] = button.darken(0.02f);
    colors[ImGuiCol_Tab] = background;
    colors[ImGuiCol_TabHovered] = foreground.lighten(0.1f);
    colors[ImGuiCol_TabActive] = foreground;
    colors[ImGuiCol_TabUnfocused] = background;
    colors[ImGuiCol_TabUnfocusedActive] = foreground;

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 0.39f);
    colors[ImGuiCol_TableRowBg] = background;
    colors[ImGuiCol_TableRowBgAlt] = foreground;

    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);

    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}
