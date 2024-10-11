#include "Fussion/OS/FileWatcher.h"

namespace Fussion {
    class LinuxFileWatcher : public FileWatcher {
    public:
        virtual void AddListener(std::function<CallbackType>) override { }
        virtual void Start() override { }
    };

    Ptr<FileWatcher> FileWatcher::Create(std::filesystem::path root)
    {
        (void)root;
        return MakePtr<LinuxFileWatcher>();
    }
}
