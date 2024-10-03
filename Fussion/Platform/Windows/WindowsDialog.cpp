#include "FussionPCH.h"
#include "Window.h"

#include "Core/Application.h"
#include "Fussion/OS/Dialog.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <ranges>
#include <shlobj_core.h>
#pragma comment(lib, "ole32.lib")

#undef MessageBox

#define WSTRING(x) std::wstring { (x).begin(), (x).end() };

namespace Fussion::Dialogs {
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

        auto handle = glfwGetWin32Window(TRANSMUTE(GLFWwindow*, Application::Self()->GetWindow().NativeHandle()));
        int answer = MessageBoxW(handle, w_message.c_str(), w_title.c_str(), a);
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
            LOG_WARNF("Unknown answer from MessageBoxW: {}", answer);
        }
        return MessageButton::Cancel;
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
        auto handle = glfwGetWin32Window(CAST(GLFWwindow*, Application::Self()->GetWindow().NativeHandle()));
        VERIFY(handle != nullptr);

        std::wstring file;
        file.resize(1024);

        using namespace std::string_literals;
        std::string filter_string;

        for (auto const& [Name, FilePatterns] : filter) {
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
        arg.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
        if (allow_multiple) {
            arg.Flags |= OFN_ALLOWMULTISELECT;
        }

        if (GetOpenFileNameW(&arg)) {
            auto buffer = std::string(file.begin(), file.end());
            // When multiple items are selected, windows returns the containing
            // folder as the first item and then only the names of the selected
            // files, seperated by 0. We assume multiple files and parse the first item
            // as the containing folder. If however, that was not the case,
            // check if the strings are empty and push that first item
            // as the only item in the selected files.
            std::vector<std::filesystem::path> strings {};
            std::filesystem::path root;
            size_t pos = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                if (buffer[i] == 0) {
                    if (i > 1 && buffer[i - 1] == 0) {
                        break;
                    }

                    if (!root.empty()) {
                        strings.emplace_back(root / buffer.substr(pos, i - pos));
                    } else [[unlikely]] {
                        root = buffer.substr(pos, i - pos);
                    }
                    pos = i + 1;
                }
            }

            // If strings is empty, it means that only one item was
            // selected from the file picker, which we parsed as
            // the root earlier.
            if (strings.empty()) {
                strings.emplace_back(root);
            }

            return strings;
        }
        return {};
    }

    std::filesystem::path ShowDirectoryPicker(std::filesystem::path const& base)
    {
        // TODO: base path is currently ignored.
        (void)base;
        LPWSTR name;
        IFileDialog* pfd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
            defer(pfd->Release());
            DWORD dw_options;
            if (SUCCEEDED(pfd->GetOptions(&dw_options))) {
                pfd->SetOptions(dw_options | FOS_PICKFOLDERS);
            }
            if (SUCCEEDED(pfd->Show(NULL))) {
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi))) {
                    defer(psi->Release());
                    if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &name))) {
                        // MessageBox(NULL, "GetIDListName() failed", NULL, NULL);
                    }
                    return name;
                }
            }
        }
        return "";
    }

    void OpenDirectory(std::filesystem::path const& path)
    {
        ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
    }
}
