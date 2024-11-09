#include "EditorPCH.h"
#include "RendererReport.h"

#include "EditorUI.h"
#include "Fussion/Rendering/Renderer.h"

using namespace Fussion;

void DrawRegistryReport(GPU::RegistryReport const& report, char const* name)
{
    if (ImGui::TreeNode(name)) {
        // Create a table for better alignment
        if (ImGui::BeginTable(name, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            // Table headers
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Field");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Value");

            // Row: num_allocated
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Allocated");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.num_allocated);

            // Row: num_kept_from_user
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Kept from User");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.num_kept_from_user);

            // Row: num_released_from_user
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Released from User");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.num_released_from_user);

            // Row: num_error
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Error");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.num_error);

            // Row: element_size
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Element Size");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.element_size);

            // End table
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}

void DrawGlobalReport(GPU::GlobalReport const& report)
{
    if (ImGui::CollapsingHeader("Global Report")) {
        DrawRegistryReport(report.adapters, "Adapters");
        DrawRegistryReport(report.devices, "Devices");
        DrawRegistryReport(report.queues, "Queues");
        DrawRegistryReport(report.pipeline_layouts, "Pipeline Layouts");
        DrawRegistryReport(report.shader_modules, "Shader Modules");
        DrawRegistryReport(report.bind_group_layouts, "Bind Group Layouts");
        DrawRegistryReport(report.bind_groups, "Bind Groups");
        DrawRegistryReport(report.command_buffers, "Command Buffers");
        DrawRegistryReport(report.render_bundles, "Render Bundles");
        DrawRegistryReport(report.render_pipelines, "Render Pipelines");
        DrawRegistryReport(report.compute_pipelines, "Compute Pipelines");
        DrawRegistryReport(report.query_sets, "Query Sets");
        DrawRegistryReport(report.buffers, "Buffers");
        DrawRegistryReport(report.textures, "Textures");
        DrawRegistryReport(report.texture_views, "Texture Views");
        DrawRegistryReport(report.samplers, "Samplers");
    }
}

void RendererReport::on_draw()
{
    EUI::window("Report", [&] {
        m_is_focused = ImGui::IsWindowFocused();

        auto& instance = Renderer::gpu_instance();
        auto report = instance.generate_global_report();

        DrawGlobalReport(report);
    });
}
