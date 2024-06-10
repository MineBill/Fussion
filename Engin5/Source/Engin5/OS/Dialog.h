#pragma once

namespace Engin5::Dialogs
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
}
