#include "Core/Core.h"
#include "Fussion/OS/Dialog.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spirv_cross/spirv.hpp>

namespace Fussion::Dialogs {
    auto shell_execute(std::string const& command) -> std::vector<std::string>
    {
        auto file = popen(command.c_str(), "r");
        defer(pclose(file));

        char buffer[1024] = {};
        fgets(buffer, sizeof(buffer), file);

        // Remove newline
        char* end = buffer;
        while (*end != '\n') {
            end++;
        }
        *end = 0;

        std::string s { buffer };
        std::vector<std::string> strings;

        int pos = 0;
        for (int i = 0; i < s.size(); ++i) {
            if (s[i] == ' ') {
                strings.push_back(s.substr(pos, i - pos));
                pos = i + 1;
            }
        }

        return strings;
    }

    class LinuxDialog {
    public:
        virtual ~LinuxDialog() = default;

        virtual auto open_file_picker(std::vector<FilePickerFilter> const& filters, bool allow_multiple) -> std::vector<std::filesystem::path> = 0;
        virtual auto open_directory_picker() -> std::filesystem::path = 0;
        virtual void show_message_box(MessageBox box) = 0;

        void set_path(std::string const& path)
        {
            m_Path = path;
        }

    protected:
        std::string m_Path;
    };

    class KDialog final : public LinuxDialog {
    public:
        auto open_directory_picker() -> std::filesystem::path override
        {
            return shell_execute(std::format("{} --getexistingdirectory", m_Path)).at(0);
        }

        auto open_file_picker(std::vector<FilePickerFilter> const& filters, bool allow_multiple) -> std::vector<std::filesystem::path> override
        {
            std::string filter_string;
            for (size_t i = 0; i < filters.size(); i++) {
                auto filter = filters[i];
                filter_string += filter.name;
                filter_string += " (";
                for (auto const& pattern : filter.file_patterns) {
                    filter_string += pattern + " ";
                }
                filter_string += ")";

                if (i != filters.size() - 1) {
                    filter_string += "|";
                }
            }

            auto string_paths = shell_execute(std::format("{0} --getopenfilename {2} . \"{1}\"", m_Path, filter_string, allow_multiple ? "--multiple" : ""));
            std::vector<std::filesystem::path> paths;
            std::ranges::copy(string_paths, std::back_inserter(paths));
            return paths;
        }

        void show_message_box(MessageBox box) override
        {
            (void)shell_execute(std::format("{} --msgbox \"{}\"", m_Path, box.message));
        }
    };

    class Zenity : public LinuxDialog {
    public:
        std::filesystem::path open_directory_picker() override
        {
            return {};
        }

        auto open_file_picker(std::vector<FilePickerFilter> const& filter, bool allow_multiple) -> std::vector<std::filesystem::path> override
        {
            (void)filter;
            (void)allow_multiple;
            return {};
        }

        void show_message_box(MessageBox box) override
        {
            (void)shell_execute(std::format("{} --msgbox \"{}\"", m_Path, box.message));
        }
    };

    namespace {
        Ptr<LinuxDialog> g_NativeDialog { nullptr };
    }

    auto get_binary_location(char const* name) -> std::optional<std::filesystem::path>
    {
        auto file = popen(std::format("/usr/bin/env whereis {}", name).c_str(), "r");
        defer(pclose(file));

        char buffer[1024] = {};
        fgets(buffer, sizeof(buffer), file);

        char* path = buffer + strlen(name) + 2;
        char* end = path;
        while (*end != ' ' && *end != '\n') {
            end++;
        }
        *end = 0;
        return path;
    }

    void create_native_dialog()
    {
        if (g_NativeDialog)
            return;
        auto kdialog = get_binary_location("kdialog");
        auto zenity = get_binary_location("zenity");
        LOG_DEBUGF("kdialog @ '{}'", kdialog.value_or("None").string());
        LOG_DEBUGF("zenity @ '{}'", zenity.value_or("None").string());

        auto const desktop = getenv("XDG_CURRENT_DESKTOP");
        LOG_DEBUGF("desktop: {}", desktop);

        if (strcmp(desktop, "KDE") == 0) {
            // Prefer kdialog on KDE
            if (kdialog) {
                g_NativeDialog = make_ptr<KDialog>();
                g_NativeDialog->set_path(kdialog->string());
            } else if (zenity) {
                g_NativeDialog = make_ptr<Zenity>();
                g_NativeDialog->set_path(zenity->string());
            }
        } else if (strcmp(desktop, "GNOME") == 0) {
            // Prefer zenity on GNOME
            if (zenity) {
                g_NativeDialog = make_ptr<Zenity>();
                g_NativeDialog->set_path(zenity->string());
            } else if (kdialog) {
                g_NativeDialog = make_ptr<KDialog>();
                g_NativeDialog->set_path(kdialog->string());
            }
        } else {
            PANIC("{} desktop not supported currently", desktop);
        }
    }

    MessageButton show_message_box(MessageBox data)
    {
        create_native_dialog();

        return MessageButton::Ok;
    }

    auto show_file_picker(std::string_view name, FilePatternList const& supported_files, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        return show_file_picker(FilePickerFilter {
                                  .name = std::string(name),
                                  .file_patterns = supported_files,
                              },
            allow_multiple);
    }

    auto show_file_picker(FilePickerFilter const& filter, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        return show_file_picker(std::vector { filter }, allow_multiple);
    }

    auto show_file_picker(std::vector<FilePickerFilter> const& filter, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        create_native_dialog();
        return g_NativeDialog->open_file_picker(filter, allow_multiple);
    }

    auto show_directory_picker(std::filesystem::path const& base) -> std::filesystem::path
    {
        (void)base;
        create_native_dialog();
        return g_NativeDialog->open_directory_picker();
    }
    
    void open_directory(std::filesystem::path const& path) {
        (void)shell_execute(std::format("xdg-open {}", path.string()));
    }
}
