#pragma once

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

    std::filesystem::path ShowFilePicker(std::string_view name, std::vector<std::string_view> supported_files);
    std::filesystem::path ShowDirectoryPicker();
}
