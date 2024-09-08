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
        void show(std::function<void(std::string)> const& callback);

        void update();
        void accept();

    private:
        bool m_show{}, m_opened{};
        std::string m_name{};
        std::function<void(std::string)> m_callback;
        bool m_show_error{};
    };

    struct Entry {
        Fussion::AssetHandle id;
        std::filesystem::path path;
        std::string string_path;
        std::string name;
        bool is_directory;
        Fsn::AssetType type;
        EditorAssetMetadata metadata;

        bool renaming{};
    };

    EDITOR_WINDOW(ContentBrowser)

    virtual void on_start() override;
    virtual void on_draw() override;

    /// Change into the directory specified by @p path.
    /// It \b MUST be a sub-path of the root project folder.
    void change_directory(std::filesystem::path const& path);

    /// Refresh the content browser by iterating again all the files of the current path.
    /// Useful after modifying a file.
    void refresh_contents();

private:
    void import_file(std::filesystem::path const& path);

    std::vector<Entry> m_entries;
    std::set<Fussion::AssetHandle> m_selection{};

    // Root path to the project, absolute.
    std::filesystem::path m_root;
    // Current path of the content browser view, absolute.
    std::filesystem::path m_current_path;
    // Current path of the content browser view, relative to the root.
    std::filesystem::path m_relative_to_root;
    std::string m_relative_to_root_string_path{};

    f32 m_padding{ 8 }, m_thumbnail_size{ 64 };

    Fsn::Dialogs::FilePickerFilter m_import_filter;
    std::unordered_map<std::string, Fsn::AssetType> m_file_types;

    NamePopup m_name_popup{};
};
