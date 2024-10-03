#include "EditorPCH.h"
#include "RendererReport.h"

#include "EditorUI.h"
#include "Fussion/Rendering/Renderer.h"

using namespace Fussion;

void draw_registry_report(GPU::RegistryReport const& report, char const* name)
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
            ImGui::Text("%zu", report.NumAllocated);

            // Row: num_kept_from_user
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Kept from User");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.NumKeptFromUser);

            // Row: num_released_from_user
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Released from User");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.NumReleasedFromUser);

            // Row: num_error
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Num Error");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.NumError);

            // Row: element_size
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Element Size");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", report.ElementSize);

            // End table
            ImGui::EndTable();
        }
    }
}

void draw_global_report(GPU::GlobalReport const& report)
{
    if (ImGui::CollapsingHeader("Global Report")) {
        // Draw each section of the GlobalReport
        draw_registry_report(report.Adapters, "Adapters");
        draw_registry_report(report.Devices, "Devices");
        draw_registry_report(report.Queues, "Queues");
        draw_registry_report(report.PipelineLayouts, "Pipeline Layouts");
        draw_registry_report(report.ShaderModules, "Shader Modules");
        draw_registry_report(report.BindGroupLayouts, "Bind Group Layouts");
        draw_registry_report(report.BindGroups, "Bind Groups");
        draw_registry_report(report.CommandBuffers, "Command Buffers");
        draw_registry_report(report.RenderBundles, "Render Bundles");
        draw_registry_report(report.RenderPipelines, "Render Pipelines");
        draw_registry_report(report.ComputePipelines, "Compute Pipelines");
        draw_registry_report(report.QuerySets, "Query Sets");
        draw_registry_report(report.Buffers, "Buffers");
        draw_registry_report(report.Textures, "Textures");
        draw_registry_report(report.TextureViews, "Texture Views");
        draw_registry_report(report.Samplers, "Samplers");
    }
}

void RendererReport::OnDraw()
{
    EUI::window("Report", [&] {
        m_IsFocused = ImGui::IsWindowFocused();

        auto& instance = Renderer::GPUInstance();
        auto report = instance.GenerateGlobalReport();

        draw_global_report(report);
    });
}
