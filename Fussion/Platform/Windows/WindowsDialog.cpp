#include "Window.h"
#include "Fussion/OS/Dialog.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <ranges>
#include <GLFW/glfw3native.h>

#undef MessageBox

#define WSTRING(x) std::wstring{(x).begin(), (x).end()};

namespace Fussion::Dialogs
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

        auto const handle = glfwGetWin32Window(TRANSMUTE(GLFWwindow*, Application::Instance()->GetWindow().NativeHandle()));
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
        auto handle = glfwGetWin32Window(CAST(GLFWwindow*, Application::Instance()->GetWindow().NativeHandle()));
        VERIFY(handle != nullptr);

        std::wstring file;
        file.resize(256);

        using namespace std::string_literals;
        std::string filter_string;

        for (const auto& [Name, FilePatterns] : filter) {
            filter_string += Name + "\0"s;

            for (size_t i = 0; i < FilePatterns.size(); i++) {
                filter_string += FilePatterns[i];
                if (FilePatterns.size() > 1 && i < FilePatterns.size() - 1) {
                    filter_string += ";"s;
                }
            }
            filter_string += "\0"s;
        }

        LOG_DEBUGF("Using the following for dialog filter !{}!", filter_string);

        std::wstring const w_filter = WSTRING(filter_string);
        OPENFILENAMEW arg;
        ZeroMemory(&arg, sizeof(arg));
        arg.lStructSize = sizeof(OPENFILENAMEW);
        arg.hwndOwner = handle;
        arg.lpstrFile = file.data();
        arg.nMaxFile = CAST(DWORD, file.size());
        arg.lpstrFilter = w_filter.c_str();
        arg.nFilterIndex = 1;
        arg.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&arg)) {
            auto s = std::string(file.begin(), file.end());
            std::erase(s, 0);
            return s;
        }
        return "";
    }

    std::filesystem::path ShowDirectoryPicker()
    {
        return "";
    }
}