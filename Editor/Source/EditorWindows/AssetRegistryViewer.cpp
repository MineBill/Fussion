#include "EditorPCH.h"
#include "AssetRegistryViewer.h"

#include "EditorUI.h"

#include <fmt/format.h>

void AssetRegistryViewer::OnDraw()
{
    EUI::Window("Asset Registry Viewer", [&] {
        auto registry = Project::AssetManager()->GetRegistry().UnsafePtr();

        ImGui::BeginTable("registry_table", 6);
        ImGui::TableSetupColumn("Handle");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Virtual");
        ImGui::TableSetupColumn("Load State");
        ImGui::TableSetupColumn("Path");
        ImGui::TableHeadersRow();

        for (auto const& [id, metadata] : *registry) {
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", fmt::format("{}", id).c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", metadata.Name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", magic_enum::enum_name(metadata.Type).data());

            ImGui::TableNextColumn();
            bool value = metadata.IsVirtual;
            ImGui::BeginDisabled();
            ImGui::Checkbox("", &value);
            ImGui::EndDisabled();

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", magic_enum::enum_name(metadata.LoadState).data());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", metadata.Path.string().c_str());
        }
        ImGui::EndTable();
    },
        { .Opened = &m_IsVisible });
}
