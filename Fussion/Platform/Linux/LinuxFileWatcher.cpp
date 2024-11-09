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
            : m_update { true }
            , m_root(std::move(root))
        { }

        virtual ~LinuxFileWatcher() override
        {
            m_update = false;
            m_thread.join();
        }

        virtual void add_listener(std::function<CallbackType> listener) override
        {
            m_listeners.subscribe(listener);
        }

        virtual void start() override
        {
            m_thread = std::thread(&LinuxFileWatcher::work, this);
        }

    private:
        static constexpr u32 IN_WATCH_MASK = IN_CREATE
            | IN_MODIFY
            | IN_MOVED_TO
            | IN_MOVED_FROM
            | IN_DELETE
            | IN_CLOSE_WRITE;

        void add_watch(std::filesystem::path const& path, u32 mask)
        {
            int wd = inotify_add_watch(m_fd, path.string().c_str(), mask);
            if (wd == -1) {
                LOG_ERRORF("Cannot watch file: {}", strerror(errno));
                return;
            }
            m_watch_descriptors[wd] = WatchData {
                .path = path,
            };
        }

        void work()
        {
            m_fd = inotify_init1(IN_NONBLOCK);
            if (m_fd == -1) {
                LOG_ERRORF("Failed to initialize inotify");
                return;
            }

            add_watch(m_root, IN_WATCH_MASK);
            for (auto const& p : std::filesystem::recursive_directory_iterator(m_root)) {
                if (p.is_directory()) {
                    add_watch(p.path(), IN_WATCH_MASK);
                }
            }

            pollfd pollFd {
                .fd = m_fd,
                .events = POLLIN,
                .revents = {}
            };

            while (m_update) {
                auto pollNum = poll(&pollFd, 1, 1);
                if (!m_update) {
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
                        while (m_update) {
                            char buf[4096]
                                __attribute__((aligned(__alignof__(inotify_event))));
                            usz const len = read(m_fd, buf, sizeof(buf));
                            if (!m_update) {
                                break;
                            }
                            if (len == cast<size_t>(-1) && errno != EAGAIN) {
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
                                    auto path = m_watch_descriptors[event->wd].path / event->name;
                                    auto type = EventType::FileAdded;
                                    // Start monitoring new directories
                                    if (event->mask & IN_ISDIR) {
                                        type = EventType::DirAdded;
                                        add_watch(path, IN_WATCH_MASK);
                                    }
                                    m_listeners.fire(path, type);
                                }
                                if (event->mask & IN_CLOSE_WRITE) {
                                    m_listeners.fire(m_watch_descriptors[event->wd].relative_to(event->name), EventType::FileModified);
                                }
                                if (event->mask & IN_MOVED_TO) {
                                    if (m_previous_event.mask & IN_MOVED_FROM) {
                                        if (m_watch_descriptors[event->wd].path == m_watch_descriptors[m_previous_event.wd].path) {
                                            m_listeners.fire(m_watch_descriptors[event->wd].relative_to(event->name), EventType::FileRenamed);
                                        } else {
                                            // LOG_DEBUGF("File moved from {} to {}",
                                            //     m_WatchDescriptors[m_PreviousEvent.WD].RelativeTo(m_PreviousEvent.Name),
                                            //     m_WatchDescriptors[event->wd].RelativeTo(event->name));
                                        }
                                    }
                                }
                                if (event->mask & IN_DELETE) {
                                    auto path = m_watch_descriptors[event->wd].path / event->name;
                                    auto type = EventType::FileDeleted;
                                    if (event->mask & IN_ISDIR) {
                                        type = EventType::DirDeleted;
                                        add_watch(path, IN_WATCH_MASK);
                                    }
                                    m_listeners.fire(path, type);
                                }

                                m_previous_event = INotifyEvent(event);
                            }
                        }
                    }
                }
            }
        }

        struct INotifyEvent {
            int wd {};     /* Watch descriptor.  */
            u32 mask {};   /* Watch mask.  */
            u32 cookie {}; /* Cookie to synchronize two events.  */
            std::string name {};

            INotifyEvent() = default;
            explicit INotifyEvent(inotify_event const* event)
                : wd(event->wd)
                , mask(event->mask)
                , cookie(event->cookie)
                , name(event->name, event->len)
            { }
        } m_previous_event;

        std::atomic<bool> m_update {};
        std::thread m_thread;
        std::filesystem::path m_root {};
        Delegate<CallbackType> m_listeners;

        int m_fd {};
        struct WatchData {
            std::filesystem::path path;

            std::string relative_to(char const* name) const
            {
                return path / name;
            }

            [[nodiscard]] std::string relative_to(std::string const& name) const
            {
                return path / name;
            }
        };
        std::unordered_map<int, WatchData> m_watch_descriptors {};
    };

    Ptr<FileWatcher> FileWatcher::create(std::filesystem::path root)
    {
        return make_ptr<LinuxFileWatcher>(root);
    }
}
