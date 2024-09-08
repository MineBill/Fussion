#include "EditorPCH.h"
#include "AssetRegistryViewer.h"

#include "EditorUI.h"

#include <fmt/format.h>

void AssetRegistryViewer::on_draw()
{
    EUI::window("Asset Registry Viewer", [&] {
        auto registry = Project::asset_manager()->registry().unsafe_ptr();

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
            ImGui::TextWrapped("%s", metadata.name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", magic_enum::enum_name(metadata.type).data());

            ImGui::TableNextColumn();
            bool value = metadata.is_virtual;
            ImGui::BeginDisabled();
            ImGui::Checkbox("", &value);
            ImGui::EndDisabled();

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", magic_enum::enum_name(metadata.load_state).data());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", metadata.path.string().c_str());
        }
        ImGui::EndTable();
    }, { .opened = &m_is_visible });
}
