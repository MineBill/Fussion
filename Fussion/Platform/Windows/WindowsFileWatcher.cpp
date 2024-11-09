#include "FussionPCH.h"

#include "Core/Core.h"
#include "Core/Delegate.h"
#include "Core/Types.h"
#include "Log/Log.h"
#include "OS/FileWatcher.h"

#include <mutex>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <fileapi.h>
#include <synchapi.h>

constexpr auto NotifyFlags = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

namespace Fussion {

    auto WindowsFileActionToEventType(DWORD action) -> FileWatcher::EventType
    {
        using enum FileWatcher::EventType;
        switch (action) {
        case FILE_ACTION_ADDED:
            return FileAdded;
        case FILE_ACTION_REMOVED:
            return FileDeleted;
        case FILE_ACTION_MODIFIED:
            return FileModified;
        case FILE_ACTION_RENAMED_OLD_NAME:
            return FileRenamed;
        case FILE_ACTION_RENAMED_NEW_NAME:
            return FileRenamed;
        default:
            break;
        }
        UNREACHABLE;
    }

    class WindowsFileWatcher final : public FileWatcher {
    public:
        explicit WindowsFileWatcher(std::filesystem::path const& path)
            : m_update { true }
            , m_root(path)
        { }

        virtual ~WindowsFileWatcher() override
        {
            m_update = false;
            m_thread.join();
            FindCloseChangeNotification(m_watch_handle);
        }

        virtual void add_listener(std::function<CallbackType> cb) override
        {
            m_listeners += cb;
        }

        virtual void start() override
        {
            m_thread = std::thread(&WindowsFileWatcher::Work, this);
        }

    private:
        void work()
        {
            // m_watch_handle = FindFirstChangeNotificationW(m_root.wstring().c_str(), true, NotifyFlags);
            m_watch_handle = CreateFile(m_root.string().c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                NULL);

            u8 buffer[1024];
            OVERLAPPED overlapped;
            overlapped.hEvent = CreateEvent(nullptr, FALSE, 0, nullptr);
            ReadDirectoryChangesW(
                m_watch_handle,
                buffer, sizeof(buffer),
                true,
                NotifyFlags,
                nullptr,
                &overlapped,
                nullptr);

            while (m_update) {
                auto status = WaitForSingleObject(overlapped.hEvent, INFINITE);
                switch (status) {
                case WAIT_OBJECT_0: {
                    DWORD bytes_transferred;
                    GetOverlappedResult(m_watch_handle, &overlapped, &bytes_transferred, FALSE);

                    auto fileInfo = TRANSMUTE(FILE_NOTIFY_INFORMATION*, buffer);

                    for (;;) {
                        std::wstring name { fileInfo->FileName, fileInfo->FileNameLength / sizeof(WCHAR) };
                        name.shrink_to_fit();
                        std::filesystem::path path { name };

                        m_listeners.fire(path, WindowsFileActionToEventType(fileInfo->Action));

                        if (fileInfo->NextEntryOffset) {
                            *((uint8_t**)&fileInfo) += fileInfo->NextEntryOffset;
                        } else {
                            break;
                        }
                    }

                    ReadDirectoryChangesW(
                        m_watch_handle,
                        buffer, sizeof(buffer),
                        true,
                        NotifyFlags,
                        nullptr,
                        &overlapped,
                        nullptr);
                    // if (!ReadDirectoryChangesW(m_watch_handle, buffer, sizeof(buffer), true, NotifyFlags, &bytes_returned, nullptr, nullptr)) {
                    //     LOG_ERRORF("ReadDirectoryChangesW failed: {}", GetLastError());
                    //     re       turn;
                    // }
                    //
                    // auto file_info = TRANSMUTE(FILE_NOTIFY_INFORMATION*, buffer);
                    // // file_info->Action
                    //
                    // {
                    //     std::scoped_lock lock(m_mutex);
                    //
                    //     m_listeners.Fire(path, WindowsFileActionToEventType(file_info->Action));
                    // }
                    // FindNextChangeNotification(m_watch_handle);
                } break;
                case WAIT_TIMEOUT:

                    break;
                default:
                    UNREACHABLE;
                    break;
                }
            }
        }

        std::atomic<bool> m_update;
        HANDLE m_watch_handle {};
        std::thread m_thread;
        std::filesystem::path m_root {};
        Delegate<CallbackType> m_listeners;
        std::mutex m_mutex {};
    };

    Ptr<FileWatcher> FileWatcher::create(std::filesystem::path root)
    {
        return make_ptr<WindowsFileWatcher>(root);
    }

}
