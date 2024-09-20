#include "EditorPCH.h"
#include "RendererReport.h"

#include "EditorUI.h"
#include "Fussion/Rendering/Renderer.h"

using namespace Fussion;

void draw_registry_report(GPU::RegistryReport const& report, const char* name)
{
    if (ImGui::CollapsingHeader(name)) {
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
    }
}

void draw_global_report(GPU::GlobalReport const& report)
{
    if (ImGui::CollapsingHeader("Global Report")) {
        // Draw each section of the GlobalReport
        draw_registry_report(report.adapters, "Adapters");
        draw_registry_report(report.devices, "Devices");
        draw_registry_report(report.queues, "Queues");
        draw_registry_report(report.pipeline_layouts, "Pipeline Layouts");
        draw_registry_report(report.shader_modules, "Shader Modules");
        draw_registry_report(report.bind_group_layouts, "Bind Group Layouts");
        draw_registry_report(report.bind_groups, "Bind Groups");
        draw_registry_report(report.command_buffers, "Command Buffers");
        draw_registry_report(report.render_bundles, "Render Bundles");
        draw_registry_report(report.render_pipelines, "Render Pipelines");
        draw_registry_report(report.compute_pipelines, "Compute Pipelines");
        draw_registry_report(report.query_sets, "Query Sets");
        draw_registry_report(report.buffers, "Buffers");
        draw_registry_report(report.textures, "Textures");
        draw_registry_report(report.texture_views, "Texture Views");
        draw_registry_report(report.samplers, "Samplers");
    }
}

void RendererReport::on_draw()
{
    EUI::window("Report", [&] {
        m_is_focused = ImGui::IsWindowFocused();

        auto& instance = Renderer::gpu_instance();
        auto report = instance.generate_global_report();

        draw_global_report(report);
    });
}
