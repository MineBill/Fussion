#include "Core/Core.h"
#include "Fussion/OS/Dialog.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spirv_cross/spirv.hpp>

namespace Fussion::Dialogs {
    std::string ShellExecute(std::string const& command)
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

        return buffer;
    }

    class LinuxDialog {
    public:
        virtual ~LinuxDialog() = default;

        virtual auto OpenFilePicker(std::vector<FilePickerFilter> const& filters) -> std::filesystem::path = 0;
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
            return ShellExecute(std::format("{} --getexistingdirectory", m_Path));
        }

        auto OpenFilePicker(std::vector<FilePickerFilter> const& filters) -> std::filesystem::path override
        {
            std::string filter_string;
            for (size_t i = 0; i < filters.size(); i++) {
                auto filter = filters[i];
                filter_string += filter.Name;
                filter_string += " (";
                for (auto const& pattern : filter.FilePatterns) {
                    filter_string += pattern + " ";
                }
                filter_string += ")";

                if (i != filters.size() - 1) {
                    filter_string += "|";
                }
            }

            return ShellExecute(std::format("{} --getopenfilename . \"{}\"", m_Path, filter_string));
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

        std::filesystem::path OpenFilePicker(std::vector<FilePickerFilter> const& filter) override
        {
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

    auto GetBinaryLocation(char const* name) -> std::optional<std::filesystem::path>
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
        auto kdialog = GetBinaryLocation("kdialog");
        auto zenity = GetBinaryLocation("zenity");
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
            // Prefet zenity on GNOME
            if (zenity) {
                g_NativeDialog = MakePtr<Zenity>();
                g_NativeDialog->SetPath(zenity->string());
            } else if (kdialog) {
                g_NativeDialog = MakePtr<KDialog>();
                g_NativeDialog->SetPath(kdialog->string());
            }
        } else {
            PANIC("{} not supported currently", desktop);
        }
    }

    MessageButton ShowMessageBox(MessageBox data)
    {
        CreateNativeDialog();

        return MessageButton::Ok;
    }

    std::filesystem::path ShowFilePicker(std::string_view name, FilePatternList const& supported_files)
    {
        return ShowFilePicker(FilePickerFilter {
            .Name = std::string(name),
            .FilePatterns = supported_files,
        });
    }

    auto ShowFilePicker(FilePickerFilter const& filter) -> std::filesystem::path
    {
        return ShowFilePicker(std::vector { filter });
    }

    auto ShowFilePicker(std::vector<FilePickerFilter> const& filters) -> std::filesystem::path
    {
        CreateNativeDialog();
        return g_NativeDialog->OpenFilePicker(filters);
    }

    auto ShowDirectoryPicker(std::filesystem::path const& path) -> std::filesystem::path
    {
        (void)path;
        CreateNativeDialog();
        return g_NativeDialog->OpenDirectoryPicker();
    }
}
