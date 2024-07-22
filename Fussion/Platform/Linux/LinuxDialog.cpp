#include "Fussion/OS/Dialog.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <spirv_cross/spirv.hpp>

#include "Core/Core.h"

namespace Fussion::Dialogs
{
    std::string ShellExecute(std::string const& command)
    {
        auto file = popen(command.c_str(), "r");
        defer(pclose(file));

        char buffer[1024] = {};
        fgets(buffer, sizeof(buffer), file);

        // Remove newline
        char* end = buffer;
        while(*end != '\n') {
            end++;
        }
        *end = 0;

        return buffer;
    }

    class LinuxDialog
    {
    public:
        virtual std::filesystem::path OpenFilePicker(std::vector<FilePickerFilter> const& filter) = 0;
        virtual std::filesystem::path OpenDirectoryPicker() = 0;
        virtual void ShowMessageBox(MessageBox box) = 0;

        void SetPath(std::string const& path)
        {
            m_Path = path;
        }
    protected:
        std::string m_Path;
    };

    class KDialog: public LinuxDialog
    {
    public:
        std::filesystem::path OpenDirectoryPicker() override
        {
            return ShellExecute(std::format("{} --getexistingdirectory", m_Path));
        }

        std::filesystem::path OpenFilePicker(std::vector<FilePickerFilter> const& filter) override
        {
            std::string filter_string;
            for (auto const& file : filter.FilePatterns) {
                filter += file;
                filter += " ";
            }
            std::string arg = std::format("{} ({})", filter.Name, filter_string);

            return ShellExecute(std::format("{} --getopenfilename . \"{}\"", m_Path, arg));
        }

        void ShowMessageBox(MessageBox box) override
        {
            (void)ShellExecute(std::format("{} --msgbox \"{}\"", m_Path, box.Message));
        }
    };

    class Zenity: public LinuxDialog
    {
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

    namespace
    {
        Ptr<LinuxDialog> g_NativeDialog{nullptr};
    }

    auto GetBinaryLocation(const char* name) -> std::optional<std::filesystem::path>
    {
        auto file = popen(std::format("/usr/bin/env whereis {}", name).c_str(), "r");
        defer(pclose(file));

        char buffer[1024] = {};
        fgets(buffer, sizeof(buffer), file);

        char* path = buffer + strlen(name) + 2;
        char* end = path;
        while(*end != ' ' && *end != '\n') {
            end++;
        }
        *end = 0;
        return path;
    }

    void CreateNativeDialog()
    {
        if (g_NativeDialog) return;
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
        return ShowFilePicker(std::vector{filter});
    }

    auto ShowFilePicker(std::vector<FilePickerFilter> const& filter) -> std::filesystem::path
    {
        CreateNativeDialog();
        return g_NativeDialog->OpenFilePicker(name, supported_files);
    }

    auto ShowDirectoryPicker() -> std::filesystem::path
    {
        CreateNativeDialog();
        return g_NativeDialog->OpenDirectoryPicker();
    }
}
