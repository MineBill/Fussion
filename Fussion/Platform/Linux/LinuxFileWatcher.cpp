#include "Fussion/OS/FileWatcher.h"

namespace Fussion {
    class LinuxFileWatcher: public FileWatcher {
    public:
        virtual void register_listener(std::function<CallbackType>) override {}
        virtual void start() override {}
    };

    Ptr<FileWatcher> FileWatcher::create(std::filesystem::path root) {
        return make_ptr<LinuxFileWatcher>();
    }
}
