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

    static Ptr<FileWatcher> Create(std::filesystem::path root);

    virtual ~FileWatcher() = default;

    virtual void RegisterListener(std::function<CallbackType>) = 0;

    virtual void Start() = 0;
};
}
