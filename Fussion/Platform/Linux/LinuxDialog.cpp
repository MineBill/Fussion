#include "Core/Core.h"
#include "Fussion/OS/Dialog.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Fussion::Dialogs {
    auto ShellExecute(std::string const& command) -> std::vector<std::string>
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

        virtual auto OpenFilePicker(std::vector<FilePickerFilter> const& filters, bool allow_multiple) -> std::vector<std::filesystem::path> = 0;
        virtual auto OpenDirectoryPicker() -> std::filesystem::path = 0;
        virtual void ShowMessageBox(MessageBox box) = 0;

        void SetPath(std::string const& path)
        {
            m_Path = path;
        }

    protected:
        std::string m_Path;
    };

    class KDialog final : public LinuxDialog {
    public:
        auto OpenDirectoryPicker() -> std::filesystem::path override
        {
            return ShellExecute(fmt::format("{} --getexistingdirectory", m_Path)).at(0);
        }

        auto OpenFilePicker(std::vector<FilePickerFilter> const& filters, bool allow_multiple) -> std::vector<std::filesystem::path> override
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

            auto string_paths = ShellExecute(std::format("{0} --getopenfilename {2} . \"{1}\"", m_Path, filter_string, allow_multiple ? "--multiple" : ""));
            std::vector<std::filesystem::path> paths;
            std::ranges::copy(string_paths, std::back_inserter(paths));
            return paths;
        }

        void ShowMessageBox(MessageBox box) override
        {
            (void)ShellExecute(std::format("{} --msgbox \"{}\"", m_Path, box.Message));
        }
    };

    class Zenity : public LinuxDialog {
    public:
        std::filesystem::path OpenDirectoryPicker() override
        {
            return {};
        }

        auto OpenFilePicker(std::vector<FilePickerFilter> const& filter, bool allow_multiple) -> std::vector<std::filesystem::path> override
        {
            (void)filter;
            (void)allow_multiple;
            return {};
        }

        void ShowMessageBox(MessageBox box) override
        {
            (void)ShellExecute(std::format("{} --msgbox \"{}\"", m_Path, box.Message));
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

    void CreateNativeDialog()
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
                g_NativeDialog = MakePtr<KDialog>();
                g_NativeDialog->SetPath(kdialog->string());
            } else if (zenity) {
                g_NativeDialog = MakePtr<Zenity>();
                g_NativeDialog->SetPath(zenity->string());
            }
        } else if (strcmp(desktop, "GNOME") == 0) {
            // Prefer zenity on GNOME
            if (zenity) {
                g_NativeDialog = MakePtr<Zenity>();
                g_NativeDialog->SetPath(zenity->string());
            } else if (kdialog) {
                g_NativeDialog = MakePtr<KDialog>();
                g_NativeDialog->SetPath(kdialog->string());
            }
        } else {
            PANIC("{} desktop not supported currently", desktop);
        }
    }

    MessageButton ShowMessageBox(MessageBox data)
    {
        CreateNativeDialog();

        return MessageButton::Ok;
    }

    auto ShowFilePicker(std::string_view name, FilePatternList const& supported_files, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        return ShowFilePicker(FilePickerFilter {
                                  .name = std::string(name),
                                  .file_patterns = supported_files,
                              },
            allow_multiple);
    }

    auto ShowFilePicker(FilePickerFilter const& filter, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        return ShowFilePicker(std::vector { filter }, allow_multiple);
    }

    auto ShowFilePicker(std::vector<FilePickerFilter> const& filter, bool allow_multiple) -> std::vector<std::filesystem::path>
    {
        CreateNativeDialog();
        return g_NativeDialog->OpenFilePicker(filter, allow_multiple);
    }

    auto ShowDirectoryPicker(std::filesystem::path const& base) -> std::filesystem::path
    {
        (void)base;
        CreateNativeDialog();
        return g_NativeDialog->OpenDirectoryPicker();
    }

    void OpenDirectory(std::filesystem::path const& path) {
        (void)ShellExecute(std::format("xdg-open {}", path.string()));
    }
}
