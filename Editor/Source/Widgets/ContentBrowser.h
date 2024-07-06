#pragma once
#include "EditorWindow.h"

#include "Fussion/Assets/Texture2D.h"
#include "Fussion/OS/Dialog.h"
#include "Fussion/Renderer/Image.h"

#include <filesystem>
#include <unordered_map>

#include "Project/EditorAssetManager.h"

class ContentBrowser: public EditorWindow
{
public:
    enum class Icon
    {
        Folder = 0,
        GenericAsset = 1,
        Back = 2,
    };

    struct Entry
    {
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

    std::unordered_map<Icon, Ref<Fsn::Texture2D>> m_Icons;
    std::vector<Entry> m_Entries;

    std::filesystem::path m_Root;
    std::filesystem::path m_CurrentPath;
    std::filesystem::path m_RelativeToRoot;

    Fsn::Dialogs::FilePickerFilter m_ImportFilter;
    std::unordered_map<std::string, Fsn::AssetType> m_FileTypes;
};

