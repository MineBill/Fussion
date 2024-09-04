#include "Fussion/OS/FileWatcher.h"

namespace Fussion {
    class LinuxFileWatcher: public FileWatcher {
    public:
        virtual void RegisterListener(std::function<CallbackType>) {}
        virtual void Start() {}
    };

    Ptr<FileWatcher> FileWatcher::Create(std::filesystem::path root) {
        return MakePtr<LinuxFileWatcher>();
    }
}
