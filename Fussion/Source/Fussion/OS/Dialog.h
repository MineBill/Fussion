#pragma once

#ifdef OS_WINDOWS
#undef MessageBox
#endif

namespace Fussion::Dialogs {
    enum class MessageAction {
        /// Display an OK button.
        Ok,
        /// Display an OK and a Cancel button.
        OkCancel,
        /// Display a Yes and a No button.
        YesNo,
        /// Display a Yes, No and a Cancel button.
        YesNoCancel,
    };

    enum class MessageButton {
        Ok,
        Yes,
        No,
        Cancel,
    };

    enum class MessageType {
        Info,
        Warning,
        Error,

        Question,
    };

    struct MessageBox {
        std::string_view title = "Title";
        std::string_view message = "Message";

        /// The type of this message box. Affects the window icon.
        /// Can be Info, Warning, Error, Question.
        MessageType type = MessageType::Info;

        /// Available actions for the user.
        MessageAction action = MessageAction::Ok;
        bool blocking = false;
    };

    /// Displays an OS message box to the user.
    /// @return The button the user pressed.
    ///         Will be one of the available buttons presented from the @p MessageAction type selected.
    MessageButton show_message_box(MessageBox data);

    using FilePatternList = std::vector<std::string>;

    struct FilePickerFilter {
        std::string name;
        FilePatternList file_patterns;
    };

    auto show_file_picker(std::string_view name, FilePatternList const& supported_files, bool allow_multiple = false) -> std::vector<std::filesystem::path>;
    auto show_file_picker(FilePickerFilter const& filter, bool allow_multiple = false) -> std::vector<std::filesystem::path>;
    auto show_file_picker(std::vector<FilePickerFilter> const& filter, bool allow_multiple = false) -> std::vector<std::filesystem::path>;

    auto show_directory_picker(std::filesystem::path const& base = std::filesystem::current_path()) -> std::filesystem::path;

    void open_directory(std::filesystem::path const& path);
}
