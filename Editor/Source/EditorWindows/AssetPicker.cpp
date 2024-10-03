#include "EditorPCH.h"

#include "AssetPicker.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Project/Project.h"

#include <Fussion/Assets/AssetManager.h>

void AssetPicker::Update()
{
    if (m_Show) {
        ImGui::OpenPopup("Asset Picker");
        m_Show = false;
    }

    bool was_open = m_Opened;

    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    EUI::modal_window("Asset Picker", [&] {
        ImGuiH::Text("Please pick an asset for '{}':", m_Member.get_name());

        ImGui::Separator();

        constexpr f32 padding { 4 }, thumbnail_size { 48 };

        auto item_size = padding + thumbnail_size + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x / item_size);
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, true);

        auto& style = EditorStyle::Style();
        for (auto const& [handle, name, virt] : m_Entries) {
            Vector2 size(thumbnail_size, thumbnail_size);

            Fussion::Texture2D* texture = style.EditorIcons[EditorIcon::GenericAsset].get();
            if (m_Type == Fussion::AssetType::Texture2D) {
                auto asset = Fussion::AssetManager::GetAsset<Fussion::Texture2D>(handle);
                if (auto ptr = asset.Get()) {
                    texture = ptr;
                }
            }
            size.x = texture->GetMetadata().Aspect() * size.y;

            EUI::image_button(texture->GetTexture().View, [&] {
                m_Member.set(m_Instance, handle);
                // TODO: Call notify methods, if available.
                m_Opened = false;
            },
                { .size = size });

            ImGui::TextUnformatted(name.data());
            ImGui::NextColumn();
        }
    },
        { .flags = flags, .opened = &m_Opened });

    if (was_open && !m_Opened) {
        m_Entries.clear();
    }
}

void AssetPicker::Show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type)
{
    m_Show = true;
    m_Member = member;
    m_Type = type;
    m_Opened = true;
    m_Instance = instance.copy();

    auto& registry = Project::AssetManager()->GetRegistry();

    registry.Access([&](EditorAssetManager::Registry const& reg) {
        for (auto const& [handle, metadata] : reg) {
            if (metadata.Type == type) {
                m_Entries.push_back({ handle, metadata.Name, metadata.IsVirtual });
            }
        }
    });
}
