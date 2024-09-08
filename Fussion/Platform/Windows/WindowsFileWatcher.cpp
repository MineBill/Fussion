#include "Core/Core.h"
#include "Core/Delegate.h"
#include "Log/Log.h"
#include "Vulkan/Common.h"

#include <Fussion/OS/FileWatcher.h>
#include <Fussion/Core/Types.h>
#include <thread>
#include <mutex>

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
        explicit WindowsFileWatcher(std::filesystem::path const& path): m_Update{ true }, m_Root(path) {}

        virtual ~WindowsFileWatcher() override
        {
            m_Update = false;
            m_Thread.join();
            FindCloseChangeNotification(m_WatchHandle);
        }

        virtual void register_listener(std::function<CallbackType> cb) override
        {
            m_Listeners += cb;
        }

        virtual void start() override
        {
            m_Thread = std::thread(&WindowsFileWatcher::Work, this);
        }

    private:
        void Work()
        {
            m_WatchHandle = FindFirstChangeNotificationW(m_Root.wstring().c_str(), true, NotifyFlags);

            while (m_Update) {
                auto status = WaitForSingleObject(m_WatchHandle, 100);
                switch (status) {
                case WAIT_OBJECT_0: {
                    u8 buffer[1024];
                    DWORD bytes_returned;
                    if (!ReadDirectoryChangesW(m_WatchHandle, buffer, sizeof(buffer), true, NotifyFlags, &bytes_returned, nullptr, nullptr)) {
                        LOG_ERRORF("ReadDirectoryChangesW failed: {}", GetLastError());
                        return;
                    }

                    auto file_info = TRANSMUTE(FILE_NOTIFY_INFORMATION*, buffer);
                    // file_info->Action
                    std::wstring name{ file_info->FileName, file_info->FileNameLength / sizeof(WCHAR) };
                    name.shrink_to_fit();
                    std::filesystem::path path{ name };

                    {
                        std::scoped_lock lock(m_Mutex);

                        m_Listeners.fire(path, WindowsFileActionToEventType(file_info->Action));
                    }

                    FindNextChangeNotification(m_WatchHandle);
                }
                break;
                case WAIT_TIMEOUT:

                    break;
                default:
                    UNREACHABLE;
                    break;
                }
            }
        }

        std::atomic<bool> m_Update;
        HANDLE m_WatchHandle{};
        std::thread m_Thread;
        std::filesystem::path m_Root{};
        Delegate<CallbackType> m_Listeners;
        std::mutex m_Mutex{};
    };

    Ptr<FileWatcher> FileWatcher::create(std::filesystem::path root)
    {
        return make_ptr<WindowsFileWatcher>(root);
    }

}
