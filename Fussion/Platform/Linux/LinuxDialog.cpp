#include "Core/Core.h"
#include "Fussion/OS/Dialog.h"
#include "Log/Log.h"

#include <dbus-cxx.h>

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

    using OpenFileFn = DBus::Path(std::string, std::string, std::map<std::string, DBus::Variant>);
    using OpenFileResponseFn = void(u32 response, std::map<std::string, DBus::Variant> data);

    class LinuxDialog {
    public:
        explicit LinuxDialog()
        {
            LOG_DEBUGF("Initializing Linux Dialog");
            DBus::set_logging_function([](
                                           char const* logger_name,
                                           SL_LogLocation const* location,
                                           SL_LogLevel const level,
                                           char const* log_string) {
                switch (level) {
                case SL_WARN:
                    LOG_WARNF("DBUS [{}]: {}", logger_name, log_string);
                    break;
                case SL_ERROR:
                    LOG_ERRORF("DBUS [{}]: {}", logger_name, log_string);
                    break;
                case SL_FATAL:
                    LOG_FATALF("DBUS [{}]: {}", logger_name, log_string);
                    break;
                default:
                    break;
                }
            });

            m_Dispatcher = DBus::StandaloneDispatcher::create();
            m_Connection = m_Dispatcher->create_connection(DBus::BusType::SESSION);
            m_DesktopProxy = m_Connection->create_object_proxy("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", DBus::ThreadForCalling::CurrentThread);
            m_OpenFileFn = m_DesktopProxy->create_method<OpenFileFn>("org.freedesktop.portal.FileChooser", "OpenFile");
        }

        virtual ~LinuxDialog() = default;

        auto OpenFilePicker(std::vector<FilePickerFilter> const& filters, bool allow_multiple, bool directory = false) -> std::vector<std::filesystem::path>
        {
            std::vector<std::filesystem::path> files {};

            std::map<std::string, DBus::Variant> options {};
            if (directory) {
                allow_multiple = false;
            }
            options["multiple"] = allow_multiple;
            options["directory"] = directory;

            auto responsePath = (*m_OpenFileFn)("", "Please select a file", options);

            auto requestProxy = m_Connection->create_object_proxy("org.freedesktop.portal.Desktop", responsePath);
            auto request = requestProxy->create_signal<OpenFileResponseFn>("org.freedesktop.portal.Request", "Response");
            request->connect([this, &files](u32 response, std::map<std::string, DBus::Variant> data) {
                if (response == 0) {
                    if (data.contains("uris")) {
                        for (auto const& file : data["uris"].to_vector<std::string>()) {
                            // We only support localhost for now.
                            if (file.starts_with("file:///")) {
                                files.emplace_back(file.substr(7));
                            } else {
                                LOG_WARNF("Got invalid URI: {}", file);
                            }
                        }
                    }
                }
                m_CompletedVariable.notify_all();
            });

            std::unique_lock lock(m_Mutex);
            m_CompletedVariable.wait(lock);

            return files;
        }

        auto OpenDirectoryPicker() -> std::filesystem::path
        {
            return OpenFilePicker({}, false, true).at(0);
        }

        virtual void ShowMessageBox(MessageBox box) = 0;

        void SetPath(std::string const& path)
        {
            m_Path = path;
        }

    protected:
        std::condition_variable m_CompletedVariable;
        std::mutex m_Mutex;
        Ref<DBus::Dispatcher> m_Dispatcher {};
        Ref<DBus::Connection> m_Connection {};
        Ref<DBus::ObjectProxy> m_DesktopProxy {};
        Ref<DBus::MethodProxy<OpenFileFn>> m_OpenFileFn {};
        std::string m_Path;
    };

    class KDialog final : public LinuxDialog {
    public:
        void ShowMessageBox(MessageBox box) override
        {
            (void)ShellExecute(std::format("{} --msgbox \"{}\"", m_Path, box.Message));
        }
    };

    class Zenity : public LinuxDialog {
    public:
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

    void OpenDirectory(std::filesystem::path const& path)
    {
        (void)ShellExecute(std::format("xdg-open {}", path.string()));
    }
}
