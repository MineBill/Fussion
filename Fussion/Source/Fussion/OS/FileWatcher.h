#pragma once
#include <Fussion/Core/Types.h>

#include <filesystem>
#include <functional>

namespace Fussion {
    class FileWatcher {
    public:
        enum class EventType {
            FileAdded,
            FileDeleted,
            FileModified,
            FileRenamed,

            DirAdded,
            DirDeleted,
        };

        using CallbackType = void(std::filesystem::path const&, EventType);

        static Ptr<FileWatcher> create(std::filesystem::path root);

        virtual ~FileWatcher() = default;

        virtual void register_listener(std::function<CallbackType>) = 0;

        virtual void start() = 0;
    };
}
