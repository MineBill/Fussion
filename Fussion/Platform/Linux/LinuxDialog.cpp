#include "Core/Core.h"
#include "Fussion/OS/Dialog.h"
#include "Log/Log.h"

#include <dbus-cxx.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Fussion::Dialogs {
    auto ShellExecute(std::string const& command) -> std::tuple<int, std::vector<std::string>>
    {
        auto file = popen(command.c_str(), "r");

        char buffer[1024] = {};
        fgets(buffer, sizeof(buffer), file);

        auto ret = pclose(file);

        // Remove newline
        char* end = buffer;
        while (*end != '\n') {
            end++;
        }
        *end = 0;

        std::string s { buffer };
        std::vector<std::string> strings;

        size_t pos = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == ' ') {
                strings.push_back(s.substr(pos, i - pos));
                pos = i + 1;
            }
        }

        return { WEXITSTATUS(ret), strings };
    }

    using OpenFileFn = DBus::Path(std::string, std::string, std::map<std::string, DBus::Variant>);
    using SaveFileFn = OpenFileFn;
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
                                           char const* log_string
                                       ) {
                (void)location;
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

            m_dispatcher = DBus::StandaloneDispatcher::create();
            m_connection = m_dispatcher->create_connection(DBus::BusType::SESSION);
            m_desktop_proxy = m_connection->create_object_proxy("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", DBus::ThreadForCalling::CurrentThread);
            m_open_file_fn = m_desktop_proxy->create_method<OpenFileFn>("org.freedesktop.portal.FileChooser", "OpenFile");
            m_save_file_fn = m_desktop_proxy->create_method<OpenFileFn>("org.freedesktop.portal.FileChooser", "SaveFile");
        }

        virtual ~LinuxDialog() = default;

        auto open_file_picker(std::vector<FilePickerFilter> const& filters, bool allow_multiple, bool directory = false, bool save_dialog = false) -> std::vector<std::filesystem::path>
        {
            std::vector<std::filesystem::path> files {};

            std::map<std::string, DBus::Variant> options {};
            if (directory) {
                allow_multiple = false;
            }
            options["multiple"] = allow_multiple;
            options["directory"] = directory;

            (void)filters;
            // std::vector<std::tuple<std::string, std::vector<std::tuple<u32, std::string>>>> portal_filters {};
            // for (auto const& filter : filters) {
            //     std::vector<std::tuple<u32, std::string>> pattern_list {};
            //     // for (auto const& pattern : filter.file_patterns) {
            //     //     pattern_list.push_back({ 0, pattern });
            //     // }
            //     (void)filter;
            //     pattern_list.emplace_back(1, "image/png");
            //     portal_filters.emplace_back(filter.name, pattern_list);
            // }
            // options["filters"] = portal_filters;
            DBus::Path response_path;
            if (save_dialog) {
                response_path = (*m_save_file_fn)("", "Please select a new file", options);
            } else {
                response_path = (*m_open_file_fn)("", "Please select a file", options);
            }

            auto request_proxy = m_connection->create_object_proxy("org.freedesktop.portal.Desktop", response_path);
            auto request = request_proxy->create_signal<OpenFileResponseFn>("org.freedesktop.portal.Request", "Response");
            (void)request->connect([this, &files](u32 response, std::map<std::string, DBus::Variant> data) {
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
                m_completed_variable.notify_all();
            });

            std::unique_lock lock(m_mutex);
            m_completed_variable.wait(lock);

            return files;
        }

        auto open_directory_picker() -> std::filesystem::path
        {
            return open_file_picker({}, false, true).at(0);
        }

        virtual MessageButton show_message_box(MessageBox box) = 0;

        void set_path(std::string const& path)
        {
            m_path = path;
        }

    protected:
        std::condition_variable m_completed_variable;
        std::mutex m_mutex;
        Ref<DBus::Dispatcher> m_dispatcher {};
        Ref<DBus::Connection> m_connection {};
        Ref<DBus::ObjectProxy> m_desktop_proxy {};
        Ref<DBus::MethodProxy<OpenFileFn>> m_open_file_fn {};
        Ref<DBus::MethodProxy<SaveFileFn>> m_save_file_fn {};
        std::string m_path;
    };

    class KDialog final : public LinuxDialog {
    public:
        virtual MessageButton show_message_box(MessageBox box) override
        {
            std::string type = "--msgbox";

            switch (box.type) {
            case MessageType::Info:
                switch (box.action) {
                case MessageAction::Ok:
                    [[fallthrough]];
                case MessageAction::OkCancel:
                    break;
                case MessageAction::YesNo:
                    type = "--yesno";
                    break;
                case MessageAction::YesNoCancel:
                    type = "--yesnocancel";
                    break;
                }
                break;
            case MessageType::Warning:
                type = "--continue-label OK --warning";
                switch (box.action) {
                case MessageAction::Ok:
                    type = "--sorry";
                    break;
                case MessageAction::OkCancel:
                    type += "continuecancel";
                    break;
                case MessageAction::YesNo:
                    type += "yesno";
                    break;
                case MessageAction::YesNoCancel:
                    type += "yesnocancel";
                    break;
                }
                break;
            case MessageType::Error:
                switch (box.action) {
                case MessageAction::Ok:
                    type = "--error";
                    break;
                case MessageAction::OkCancel:
                    [[fallthrough]];
                case MessageAction::YesNo:
                    [[fallthrough]];
                case MessageAction::YesNoCancel:
                    break;
                }
                break;
            case MessageType::Question:
                switch (box.action) {
                case MessageAction::Ok:
                    [[fallthrough]];
                case MessageAction::OkCancel:
                    break;
                case MessageAction::YesNo:
                    type = "--yesno";
                    break;
                case MessageAction::YesNoCancel:
                    type = "--yesnocancel";
                    break;
                }
                break;
            }
            auto [ret, output] = ShellExecute(std::format(R"({} {} "{}" --title "{}")", m_path, type, box.message, box.title));
            (void)output;

            switch (box.action) {
            case MessageAction::Ok:
                if (ret == 0) {
                    return MessageButton::Ok;
                }
                break;
            case MessageAction::OkCancel:
                switch (ret) {
                case 0:
                    return MessageButton::Ok;
                case 1:
                    return MessageButton::Cancel;
                default:
                    break;
                }
                break;
            case MessageAction::YesNo:
                switch (ret) {
                case 0:
                    return MessageButton::Yes;
                case 1:
                    return MessageButton::No;
                default:
                    break;
                }
                break;
            case MessageAction::YesNoCancel:
                switch (ret) {
                case 0:
                    return MessageButton::Yes;
                case 1:
                    return MessageButton::No;
                case 2:
                    return MessageButton::Cancel;
                default:
                    break;
                }
                break;
            }
            return MessageButton::Ok;
        }
    };

    class Zenity final : public LinuxDialog {
    public:
        virtual MessageButton show_message_box(MessageBox box) override
        {
            (void)box;
            return MessageButton::Ok;
        }
    };

    namespace {
        Ptr<LinuxDialog> g_native_dialog { nullptr };
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
        if (g_native_dialog)
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
                g_native_dialog = make_ptr<KDialog>();
                g_native_dialog->set_path(kdialog->string());
            } else if (zenity) {
                g_native_dialog = make_ptr<Zenity>();
                g_native_dialog->set_path(zenity->string());
            }
        } else if (strcmp(desktop, "GNOME") == 0) {
            // Prefer zenity on GNOME
            if (zenity) {
                g_native_dialog = make_ptr<Zenity>();
                g_native_dialog->set_path(zenity->string());
            } else if (kdialog) {
                g_native_dialog = make_ptr<KDialog>();
                g_native_dialog->set_path(kdialog->string());
            }
        } else {
            PANIC("{} desktop not supported currently", desktop);
        }
    }

    MessageButton show_message_box(MessageBox data)
    {
        (void)data;
        create_native_dialog();

        return g_native_dialog->show_message_box(data);
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
        return g_native_dialog->open_file_picker(filter, allow_multiple);
    }

    auto show_save_dialog(std::filesystem::path const& base_path) -> std::filesystem::path
    {
        (void)base_path;
        create_native_dialog();
        return g_native_dialog->open_file_picker({}, false, false, true)[0];
    }

    auto show_directory_picker(std::filesystem::path const& base) -> std::filesystem::path
    {
        (void)base;
        create_native_dialog();
        return g_native_dialog->open_directory_picker();
    }

    void open_directory(std::filesystem::path const& path)
    {
        (void)ShellExecute(std::format("xdg-open {}", path.string()));
    }
}
