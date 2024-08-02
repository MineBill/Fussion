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

namespace Fussion {
    class WindowsFileWatcher final : public FileWatcher {
    public:
        explicit WindowsFileWatcher(std::filesystem::path const& path): m_Update{ true }, m_Root(path) {}

        virtual ~WindowsFileWatcher() override
        {
            m_Update = false;
            // LOG_DEBUG("Joining thread");
            m_Thread.join();
            FindCloseChangeNotification(m_WatchHandle);
            // LOG_DEBUG("Joining done");
        }

        virtual void RegisterListener(std::function<CallbackType> cb) override
        {
            m_Listeners += cb;
        }

        virtual void Start() override
        {
            m_Thread = std::thread(&WindowsFileWatcher::Work, this);
        }

    private:
        void Work()
        {
            m_WatchHandle = FindFirstChangeNotificationW(m_Root.wstring().c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE);

            while (m_Update) {
                // LOG_DEBUG("Waiting for single object");
                auto status = WaitForSingleObject(m_WatchHandle, 100);
                // LOG_DEBUG("Done waiting");
                switch (status) {
                case WAIT_OBJECT_0: {
                    u8 buffer[1024];
                    DWORD bytes_returned;
                    if (!ReadDirectoryChangesW(m_WatchHandle, buffer, sizeof(buffer), false, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes_returned, nullptr, nullptr)) {
                        LOG_ERRORF("ReadDirectoryChangesW failed: {}", GetLastError());
                        return;
                    }

                    auto file_info = TRANSMUTE(FILE_NOTIFY_INFORMATION*, buffer);
                    std::wstring name{ file_info->FileName, file_info->FileNameLength / sizeof(WCHAR) };
                    name.shrink_to_fit();
                    std::filesystem::path path{ name };

                    {
                        std::scoped_lock lock(m_Mutex);

                        m_Listeners.Fire(path, EventType::FileModified);
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

    Ptr<FileWatcher> FileWatcher::Create(std::filesystem::path root)
    {
        return MakePtr<WindowsFileWatcher>(root);
    }

}
