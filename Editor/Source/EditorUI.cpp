#include "EditorUI.h"
#include "Layers/Editor.h"

namespace EUI {
    namespace Detail {
        ButtonStyle& GetButtonStyle(ButtonStyles style)
        {
            return EditorStyle::GetStyle().ButtonStyles[style];
        }

        WindowStyle& GetWindowStyle(WindowStyles style)
        {
            return EditorStyle::GetStyle().WindowStyles[style];
        }
    }

    void AssetProperty(meta_hpp::class_type class_type, meta_hpp::uvalue data) {
        auto m_Handle = class_type.get_member("m_Handle");

        ImGui::TextUnformatted("Asset Reference:");
        ImGui::SetNextItemAllowOverlap();
        Vector2 pos = ImGui::GetCursorPos();

        auto handle = m_Handle.get(data).as<Fussion::AssetHandle>();
        auto asset_metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(handle);
        ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::BoldSmall]);
        ImGui::Button(std::format("{}", asset_metadata.Name.data()).data(), Vector2(64, 64));
        ImGui::PopFont();

        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
            Editor::Get().OpenAsset(asset_metadata.Handle);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Clear")) {
                m_Handle.set(data, Fussion::AssetHandle(0));
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::GetDragDropPayload();
            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                auto incoming_handle = CAST(Fussion::AssetHandle*, payload->Data);
                auto incoming_metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(*incoming_handle);

                if (incoming_metadata.Type == asset_metadata.Type && ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                    m_Handle.set(data, *incoming_handle);
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SetCursorPos(pos + Vector2(2, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(0, 0));
        ImageButton(EditorStyle::GetStyle().EditorIcons[EditorIcon::Search], [&] {
            auto asset_type = class_type.get_method("GetType").invoke(data).as<Fussion::AssetType>();
            Editor::GenericAssetPicker.Show(m_Handle, data, asset_type);
        }, { .Size = Vector2{ 16, 16 } });
        ImGui::PopStyleVar();
    }
}
