#include "EditorPCH.h"
#include "EditorUI.h"
#include "Layers/Editor.h"

namespace EUI {
    namespace Detail {
        ButtonStyle& get_button_style(ButtonStyles style)
        {
            return EditorStyle::Style().ButtonStyles[style];
        }

        WindowStyle& get_window_style(WindowStyles style)
        {
            return EditorStyle::Style().WindowStyles[style];
        }
    }

    bool asset_property(meta_hpp::class_type class_type, meta_hpp::uvalue data)
    {
        bool modified {false};
        auto m_Handle = class_type.get_member("m_Handle");
        auto GetType = class_type.get_method("GetType");
        auto asset_type = GetType(data).as<Fussion::AssetType>();

        ImGui::TextUnformatted("Asset Reference:");
        ImGui::SetNextItemAllowOverlap();
        Vector2 pos = ImGui::GetCursorPos();

        auto handle = m_Handle.get(data).as<Fussion::AssetHandle>();
        auto asset_metadata = Project::AssetManager()->GetMetadata(handle);

        ImGui::PushFont(EditorStyle::Style().Fonts[EditorFont::BoldSmall]);
        if (asset_metadata.IsValid()) {
            ImGui::Button(std::format("{}", asset_metadata.Name.data()).data(), Vector2(64, 64));

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                Editor::Self().OpenAsset(asset_metadata.Handle);
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Clear")) {
                    m_Handle.set(data, Fussion::AssetHandle(0));
                    modified = true;
                }
                ImGui::EndPopup();
            }
        } else {
            if (m_Handle.get(data).as<Fussion::Uuid>() == 0) {
                ImGui::Button("[Empty]", Vector2(64, 64));
            } else {
                ImGui::Button("[Broken reference]", Vector2(64, 64));
            }
        }
        ImGui::PopFont();

        if (ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::GetDragDropPayload();
            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                auto incoming_handle = CAST(Fussion::AssetHandle*, payload->Data);
                auto incoming_metadata = Project::AssetManager()->GetMetadata(*incoming_handle);

                if (incoming_metadata.Type == asset_type && ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                    m_Handle.set(data, *incoming_handle);
                    modified = true;
                }
            }

            ImGui::EndDragDropTarget();
        }

        auto old_pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(pos + Vector2(2, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(0, 0));
        image_button(EditorStyle::Style().EditorIcons[EditorIcon::Search], [&] {
            auto asset_type = class_type.get_method("GetType").invoke(data).as<Fussion::AssetType>();
            Editor::GenericAssetPicker.Show(m_Handle, data, asset_type);
        }, { .size = Vector2{ 16, 16 } });
        ImGui::PopStyleVar();
        ImGui::SetCursorPos(old_pos);
        return modified;
    }
}
