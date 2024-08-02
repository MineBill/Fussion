#pragma once
#include "EditorWindow.h"
#include "Project/EditorAssetManager.h"

#include "Fussion/OS/Dialog.h"

#include <filesystem>
#include <unordered_map>

class ContentBrowser : public EditorWindow {
public:
    struct Entry {
        std::filesystem::path Path;
        std::string StringPath;
        std::string Name;
        bool IsDirectory;
        Fsn::AssetType Type;
        AssetMetadata Metadata;
    };

    EDITOR_WINDOW(ContentBrowser)

    void OnStart() override;
    void OnDraw() override;

    void ChangeDirectory(std::filesystem::path path);
    void Refresh();

private:
    void ImportFile(std::filesystem::path const& path);

    std::vector<Entry> m_Entries;

    std::filesystem::path m_Root;
    std::filesystem::path m_CurrentPath;
    std::filesystem::path m_RelativeToRoot;

    f32 m_Padding{ 8 }, m_ThumbnailSize{ 96 };

    Fsn::Dialogs::FilePickerFilter m_ImportFilter;
    std::unordered_map<std::string, Fsn::AssetType> m_FileTypes;

};
