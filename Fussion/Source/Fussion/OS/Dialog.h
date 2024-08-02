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
        std::string_view Title = "Title";
        std::string_view Message = "Message";

        /// The type of this message box. Affects the window icon.
        /// Can be Info, Warning, Error, Question.
        MessageType Type = MessageType::Info;

        /// Available actions for the user.
        MessageAction Action = MessageAction::Ok;
        bool Blocking = false;
    };

    /// Displays an OS message box to the user.
    /// @return The button the user pressed.
    ///         Will be one of the available buttons presented from the @p MessageAction type selected.
    MessageButton ShowMessageBox(MessageBox data);

    using FilePatternList = std::vector<std::string>;

    struct FilePickerFilter {
        std::string Name;
        FilePatternList FilePatterns;
    };

    auto ShowFilePicker(std::string_view name, FilePatternList const& supported_files) -> std::filesystem::path;
    auto ShowFilePicker(FilePickerFilter const& filter) -> std::filesystem::path;
    auto ShowFilePicker(std::vector<FilePickerFilter> const& filter) -> std::filesystem::path;

    auto ShowDirectoryPicker(std::filesystem::path const& base = std::filesystem::current_path()) -> std::filesystem::path;
}
