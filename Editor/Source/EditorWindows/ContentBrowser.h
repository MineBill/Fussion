#pragma once
#include "EditorWindow.h"
#include "Project/EditorAssetManager.h"

#include "Fussion/OS/Dialog.h"

#include <filesystem>
#include <unordered_map>

class ContentBrowser final : public EditorWindow {
public:
    class NamePopup final {
    public:
        void Show(std::function<void(std::string)> const& callback);

        void Update();
        void Accept();

    private:
        bool m_Show{}, m_Opened{};
        std::string m_Name{};
        std::function<void(std::string)> m_Callback;
        bool m_ShowError{};
    };

    struct Entry {
        Fussion::AssetHandle Id;
        std::filesystem::path Path;
        std::string StringPath;
        std::string Name;
        bool IsDirectory;
        Fsn::AssetType Type;
        EditorAssetMetadata Metadata;

        bool Renaming{};
    };

    EDITOR_WINDOW(ContentBrowser)

    virtual void OnStart() override;
    virtual void OnDraw() override;

    /// Change into the directory specified by @p path.
    /// It \b MUST be a sub-path of the root project folder.
    void ChangeDirectory(std::filesystem::path const& path);

    /// Refresh the content browser by iterating again all the files of the current path.
    /// Useful after modifying a file.
    void Refresh();

private:
    void ImportFile(std::filesystem::path const& path);

    std::vector<Entry> m_Entries;
    std::set<Fussion::AssetHandle> m_Selection{};

    // Root path to the project, absolute.
    std::filesystem::path m_Root;
    // Current path of the content browser view, absolute.
    std::filesystem::path m_CurrentPath;
    // Current path of the content browser view, relative to the root.
    std::filesystem::path m_RelativeToRoot;
    std::string m_RelativeToRootStringPath{};

    f32 m_Padding{ 8 }, m_ThumbnailSize{ 64 };

    Fsn::Dialogs::FilePickerFilter m_ImportFilter;
    std::unordered_map<std::string, Fsn::AssetType> m_FileTypes;

    NamePopup m_NamePopup{};
};
