#include "EditorPCH.h"
#include "EditorApplication.h"
#include <Fussion/OS/Args.h>
#if defined(FSN_LIVEPP_ENABLED)
#include "LivePP/API/x64/LPP_API_x64_CPP.h"
#endif

int main(int argc, char** argv)
{
#if defined(FSN_LIVEPP_ENABLED)
    lpp::LppDefaultAgent lpp_agent = lpp::LppCreateDefaultAgent(nullptr, L"Source/LivePP");
    if (!lpp::LppIsValidDefaultAgent(&lpp_agent)) {
        return 1;
    }
    lpp_agent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr, nullptr);
#endif

    Fussion::Args::Collect(argc, argv);
    EditorApplication editor{};
    editor.Run();

#if defined(FSN_LIVEPP_ENABLED)
    lpp::LppDestroyDefaultAgent(&lpp_agent);
#endif
    return 0;
}
