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

        DirAdded,
        DirDeleted,
    };

    static Ptr<FileWatcher> Create(std::filesystem::path root);

    virtual ~FileWatcher() = default;

    using CallbackType = void(std::filesystem::path const&, EventType);
    virtual void RegisterListener(std::function<CallbackType>) = 0;

    virtual void Start() = 0;
};
}
