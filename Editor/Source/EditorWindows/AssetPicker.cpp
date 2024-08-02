#include "AssetPicker.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Project/Project.h"

void AssetPicker::Update()
{
    if (m_Show) {
        ImGui::OpenPopup("Asset Picker");
        m_Show = false;
    }

    bool was_open = m_Opened;

    EUI::ModalWindow("Asset Picker", [&] {
        ImGuiH::Text("Please pick an asset for '{}':", m_Member.get_name());

        ImGui::Separator();
        for (auto const& handle : m_ViableHandles) {
            EUI::Button(std::format("Asset: {}", handle), [&] {
                m_Member.set(m_Instance, handle);
                m_Opened = false;
            });
        }
    }, { .Flags = ImGuiPopupFlags_None, .Opened = &m_Opened });

    if (was_open && !m_Opened) {
        m_ViableHandles.clear();
    }
}

void AssetPicker::Show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type)
{
    m_Show = true;
    m_Member = member;
    m_Type = type;
    m_Opened = true;
    m_Instance = instance.copy();

    for (auto const& [handle, metadata] : Project::ActiveProject()->GetAssetManager()->GetRegistry()) {
        if (metadata.Type == type) {
            m_ViableHandles.push_back(handle);
        }
    }
}
