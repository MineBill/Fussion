#include "Window.h"
#include "Engin5/OS/Dialog.h"
#include "Core/Application.h"

#include <GLFW//glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#undef MessageBox

#define WSTRING(x) std::wstring{(x).begin(), (x).end()};

namespace Engin5::Dialogs
{
    MessageButton ShowMessageBox(MessageBox data)
    {
        std::wstring const w_message = WSTRING(data.Message);
        std::wstring const w_title = WSTRING(data.Title);

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

        auto const handle = glfwGetWin32Window(transmute(GLFWwindow*, Application::Instance()->GetWindow().NativeHandle()));
        int const answer = MessageBoxW(handle, w_message.c_str(), w_title.c_str(), a);
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

    std::filesystem::path ShowFilePicker(std::string_view name, std::vector<std::string_view> supported_files)
    {
        auto handle = glfwGetWin32Window(cast(GLFWwindow*, Application::Instance()->GetWindow().NativeHandle()));
        EASSERT(handle != nullptr);

        std::wstring file;
        file.resize(256);

        using namespace std::string_literals;
        std::string filter;
        filter += name;
        for (auto const& pattern : supported_files) {
            filter += "\0"s;
            filter += pattern;
        }
        filter += "\0"s;

        std::wstring const w_filter = WSTRING(filter);
        OPENFILENAMEW arg;
        ZeroMemory(&arg, sizeof(arg));
        arg.lStructSize = sizeof(OPENFILENAMEW);
        arg.hwndOwner = handle;
        arg.lpstrFile = file.data();
        arg.nMaxFile = file.size();
        arg.lpstrFilter = w_filter.c_str();
        arg.nFilterIndex = 1;
        arg.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&arg)) {
            return std::string(file.begin(), file.end());
        }
        return "";
    }

    std::filesystem::path ShowDirectoryPicker()
    {
        return "";
    }
}