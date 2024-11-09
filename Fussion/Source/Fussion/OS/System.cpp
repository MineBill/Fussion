#include "System.h"
#include "Core/Core.h"

namespace Fussion {
    System::SystemType System::current_system_type()
    {
#ifdef OS_LINUX
        return SystemType::Linux;
#elifdef OS_WINDOWS
        return SystemType::Linux;
#else
        PANIC("Unknown System Type");
#endif
    }
}
