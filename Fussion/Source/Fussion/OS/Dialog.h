#pragma once

#ifdef OS_WINDOWS
#undef MessageBox
#endif

namespace Fussion::Dialogs
{
    enum class MessageAction
    {
        Ok,
        OkCancel,
        YesNo,
        YesNoCancel,
    };

    enum class MessageButton
    {
        Ok,
        Yes,
        No,
        Cancel,
    };

    enum class MessageType
    {
        Info,
        Warning,
        Error,

        Question,
    };

    struct MessageBox
    {
        std::string_view Title = "Title";
        std::string_view Message = "Message";
        MessageType Type = MessageType::Info;
        MessageAction Action = MessageAction::Ok;
        bool Blocking = false;
    };

    MessageButton ShowMessageBox(MessageBox data);

    using FilePatternList = std::vector<std::string>;
    struct FilePickerFilter {
        std::string Name;
        FilePatternList FilePatterns;
    };

    auto ShowFilePicker(std::string_view name, FilePatternList const& supported_files) -> std::filesystem::path;
    auto ShowFilePicker(FilePickerFilter const& filter) -> std::filesystem::path;
    auto ShowFilePicker(std::vector<FilePickerFilter> const& filter) -> std::filesystem::path;

    auto ShowDirectoryPicker() -> std::filesystem::path;
}
