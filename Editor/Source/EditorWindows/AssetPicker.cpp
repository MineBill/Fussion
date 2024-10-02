#include "EditorPCH.h"

#include "AssetPicker.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Project/Project.h"

#include <Fussion/Assets/AssetManager.h>

void AssetPicker::update()
{
    if (m_show) {
        ImGui::OpenPopup("Asset Picker");
        m_show = false;
    }

    bool was_open = m_opened;

    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    EUI::modal_window("Asset Picker", [&] {
        ImGuiH::Text("Please pick an asset for '{}':", m_member.get_name());

        ImGui::Separator();

        constexpr f32 padding{ 4 }, thumbnail_size{ 48 };

        auto item_size = padding + thumbnail_size + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x / item_size);
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, true);

        auto& style = EditorStyle::get_style();
        for (auto const& [handle, name, virt] : m_entries) {
            Vector2 size(thumbnail_size, thumbnail_size);

            Fussion::Texture2D* texture = style.editor_icons[EditorIcon::GenericAsset].get();
            if (m_type == Fussion::AssetType::Texture2D) {
                auto asset = Fussion::AssetManager::get_asset<Fussion::Texture2D>(handle);
                if (auto ptr = asset.get()) {
                    texture = ptr;
                }
            }
            size.x = texture->metadata().aspect() * size.y;

            EUI::image_button(texture->image().view, [&] {
                m_member.set(m_instance, handle);
                // TODO: Call notify methods, if available.
                m_opened = false;
            }, { .size = size });

            ImGui::TextUnformatted(name.data());
            ImGui::NextColumn();
        }
    }, { .flags = flags, .opened = &m_opened });

    if (was_open && !m_opened) {
        m_entries.clear();
    }
}

void AssetPicker::show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type)
{
    m_show = true;
    m_member = member;
    m_type = type;
    m_opened = true;
    m_instance = instance.copy();

    auto& registry = Project::asset_manager()->registry();

    registry.access([&](EditorAssetManager::Registry const& reg) {
        for (auto const& [handle, metadata] : reg) {
            if (metadata.type == type) {
                m_entries.push_back({ handle, metadata.name, metadata.is_virtual });
            }
        }
    });
}
