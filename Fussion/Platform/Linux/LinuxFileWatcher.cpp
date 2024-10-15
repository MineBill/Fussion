#include "Core/Core.h"
#include "Fussion/Core/Delegate.h"
#include "Fussion/OS/FileWatcher.h"
#include "Log/Log.h"

#include <cerrno>
#include <cstring>
#include <thread>
#include <utility>

#include <sys/inotify.h>
#include <sys/poll.h>

namespace Fussion {
    class LinuxFileWatcher final : public FileWatcher {
    public:
        explicit LinuxFileWatcher(std::filesystem::path root)
            : m_Root(std::move(root))
            , m_Update { true }
        { }

        virtual ~LinuxFileWatcher() override
        {
            m_Update = false;
            m_Thread.join();
        }

        virtual void AddListener(std::function<CallbackType> listener) override
        {
            m_Listeners.Subscribe(listener);
        }

        virtual void Start() override
        {
            m_Thread = std::thread(&LinuxFileWatcher::Work, this);
        }

    private:
        static constexpr u32 IN_WATCH_MASK = IN_CREATE
            | IN_MODIFY
            | IN_MOVED_TO
            | IN_MOVED_FROM
            | IN_DELETE
            | IN_CLOSE_WRITE;

        void AddWatch(std::filesystem::path const& path, u32 mask)
        {
            int wd = inotify_add_watch(m_FD, path.string().c_str(), mask);
            if (wd == -1) {
                LOG_ERRORF("Cannot watch file: {}", strerror(errno));
                return;
            }
            m_WatchDescriptors[wd] = WatchData {
                .Path = path,
            };
        }

        void Work()
        {
            m_FD = inotify_init1(IN_NONBLOCK);
            if (m_FD == -1) {
                LOG_ERRORF("Failed to initialize inotify");
                return;
            }

            AddWatch(m_Root, IN_WATCH_MASK);
            for (auto const& p : std::filesystem::recursive_directory_iterator(m_Root)) {
                if (p.is_directory()) {
                    AddWatch(p.path(), IN_WATCH_MASK);
                }
            }

            pollfd pollFd {
                .fd = m_FD,
                .events = POLLIN,
                .revents = {}
            };

            while (m_Update) {
                auto pollNum = poll(&pollFd, 1, 1);
                if (!m_Update) {
                    return;
                }
                if (pollNum == -1) {
                    if (errno == EINTR) {
                        continue;
                    }
                    LOG_ERRORF("Error on poll");
                    return;
                }

                if (pollNum > 0) {
                    if (pollFd.revents & POLLIN) {
                        inotify_event const* event;
                        while (m_Update) {
                            char buf[4096]
                                __attribute__((aligned(__alignof__(inotify_event))));
                            usz const len = read(m_FD, buf, sizeof(buf));
                            if (!m_Update) {
                                break;
                            }
                            if (len == -1 && errno != EAGAIN) {
                                LOG_ERRORF("Failed on read");
                                return;
                            }

                            if (len <= 0)
                                break;

                            for (char* ptr = buf; ptr < buf + len;
                                 ptr += sizeof(inotify_event) + event->len) {

                                event = TRANSMUTE(inotify_event const*, ptr);

                                /* Print event type. */
                                if (event->mask & IN_CREATE) {
                                    auto path = m_WatchDescriptors[event->wd].Path / event->name;
                                    auto type = EventType::FileAdded;
                                    // Start monitoring new directories
                                    if (event->mask & IN_ISDIR) {
                                        type = EventType::DirAdded;
                                        AddWatch(path, IN_WATCH_MASK);
                                    }
                                    m_Listeners.Fire(path, type);
                                }
                                if (event->mask & IN_CLOSE_WRITE) {
                                    m_Listeners.Fire(m_WatchDescriptors[event->wd].RelativeTo(event->name), EventType::FileModified);
                                }
                                if (event->mask & IN_MOVED_TO) {
                                    if (m_PreviousEvent.Mask & IN_MOVED_FROM) {
                                        if (m_WatchDescriptors[event->wd].Path == m_WatchDescriptors[m_PreviousEvent.WD].Path) {
                                            m_Listeners.Fire(m_WatchDescriptors[event->wd].RelativeTo(event->name), EventType::FileRenamed);
                                        } else {
                                            // LOG_DEBUGF("File moved from {} to {}",
                                            //     m_WatchDescriptors[m_PreviousEvent.WD].RelativeTo(m_PreviousEvent.Name),
                                            //     m_WatchDescriptors[event->wd].RelativeTo(event->name));
                                        }
                                    }
                                }
                                if (event->mask & IN_DELETE) {
                                    auto path = m_WatchDescriptors[event->wd].Path / event->name;
                                    auto type = EventType::FileDeleted;
                                    if (event->mask & IN_ISDIR) {
                                        type = EventType::DirDeleted;
                                        AddWatch(path, IN_WATCH_MASK);
                                    }
                                    m_Listeners.Fire(path, type);
                                }

                                m_PreviousEvent = INotifyEvent(event);
                            }
                        }
                    }
                }
            }
        }

        struct INotifyEvent {
            int WD {};     /* Watch descriptor.  */
            u32 Mask {};   /* Watch mask.  */
            u32 Cookie {}; /* Cookie to synchronize two events.  */
            std::string Name {};

            INotifyEvent() = default;
            explicit INotifyEvent(inotify_event const* event)
                : WD(event->wd)
                , Mask(event->mask)
                , Cookie(event->cookie)
                , Name(event->name, event->len)
            { }
        } m_PreviousEvent;

        std::atomic<bool> m_Update {};
        std::thread m_Thread;
        std::filesystem::path m_Root {};
        Delegate<CallbackType> m_Listeners;

        int m_FD {};
        struct WatchData {
            std::filesystem::path Path;

            std::string RelativeTo(char const* name) const
            {
                return Path / name;
            }

            [[nodiscard]] std::string RelativeTo(std::string const& name) const
            {
                return Path / name;
            }
        };
        std::unordered_map<int, WatchData> m_WatchDescriptors {};
    };

    Ptr<FileWatcher> FileWatcher::Create(std::filesystem::path root)
    {
        return MakePtr<LinuxFileWatcher>(root);
    }
}
