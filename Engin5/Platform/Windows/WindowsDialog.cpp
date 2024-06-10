#include "Window.h"
#include "Engin5/OS/Dialog.h"
#include "Windows.h"
#include "Core/Application.h"
#undef MessageBox

#define WSTRING(x) std::wstring{(x).begin(), (x).end()};

namespace Engin5::Dialogs
{
    MessageButton ShowMessageBox(MessageBox data)
    {
        std::wstring w_message = WSTRING(data.Message);
        std::wstring w_title = WSTRING(data.Title);

        UINT a = MB_OK;
        switch (data.Action) {
        case MessageAction::Ok:
            a = MB_OK;
            break;
        case MessageAction::OkCancel:
            a = MB_OKCANCEL;
            break;
        case MessageAction::YesNo:
            a = MB_YESNO;
            break;
        case MessageAction::YesNoCancel:
            a = MB_YESNOCANCEL;
            break;
        }

        switch (data.Type) {
        case MessageType::Info:
            a |= MB_ICONINFORMATION;
            break;
        case MessageType::Warning:
            a |= MB_ICONWARNING;
            break;
        case MessageType::Error:
            a |= MB_ICONERROR;
            break;
        case MessageType::Question:
            a |= MB_ICONQUESTION;
            break;
        }

        a |= MB_SYSTEMMODAL;

        auto handle = Application::Instance()->GetWindow().NativeHandle();
        int answer = MessageBoxW(*cast(HWND*, handle), w_message.c_str(), w_title.c_str(), a);
        switch (answer) {
        case IDOK:
            return MessageButton::Ok;
        case IDYES:
            return MessageButton::Yes;
        case IDNO:
            return MessageButton::No;
        case IDCANCEL:
            return MessageButton::Cancel;
        default:
            PANIC("Unkown ansewer from MessageBoxW: {}", answer);
        }
        return MessageButton::Ok;
    }
}