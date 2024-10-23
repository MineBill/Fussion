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
        ImGui::TreePop();
    }
}

void DrawGlobalReport(GPU::GlobalReport const& report)
{
    if (ImGui::CollapsingHeader("Global Report")) {
        DrawRegistryReport(report.Adapters, "Adapters");
        DrawRegistryReport(report.Devices, "Devices");
        DrawRegistryReport(report.Queues, "Queues");
        DrawRegistryReport(report.PipelineLayouts, "Pipeline Layouts");
        DrawRegistryReport(report.ShaderModules, "Shader Modules");
        DrawRegistryReport(report.BindGroupLayouts, "Bind Group Layouts");
        DrawRegistryReport(report.BindGroups, "Bind Groups");
        DrawRegistryReport(report.CommandBuffers, "Command Buffers");
        DrawRegistryReport(report.RenderBundles, "Render Bundles");
        DrawRegistryReport(report.RenderPipelines, "Render Pipelines");
        DrawRegistryReport(report.ComputePipelines, "Compute Pipelines");
        DrawRegistryReport(report.QuerySets, "Query Sets");
        DrawRegistryReport(report.Buffers, "Buffers");
        DrawRegistryReport(report.Textures, "Textures");
        DrawRegistryReport(report.TextureViews, "Texture Views");
        DrawRegistryReport(report.Samplers, "Samplers");
    }
}

void RendererReport::OnDraw()
{
    EUI::Window("Report", [&] {
        m_IsFocused = ImGui::IsWindowFocused();

        auto& instance = Renderer::GPUInstance();
        auto report = instance.GenerateGlobalReport();

        DrawGlobalReport(report);
    });
}
