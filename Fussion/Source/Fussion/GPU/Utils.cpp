#include "Utils.h"

#include "C:/Users/Jonathan/scoop/apps/renderdoc/1.34/renderdoc_app.h"
#include "FussionPCH.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace Fussion::GPU::Utils {
    RENDERDOC_API_1_1_2* g_Rdoc_Api = nullptr;

    void RenderDoc::initialize()
    {
        if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
            auto renderdoc_get_api = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
            int ret = renderdoc_get_api(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&g_Rdoc_Api));
            assert(ret == 1);

            LOG_INFO("RenderDoc connection active");
        } else {
            LOG_INFO("No RenderDoc connection.");
        }
    }

    void RenderDoc::start_capture()
    {
        if (g_Rdoc_Api) {
            g_Rdoc_Api->StartFrameCapture(nullptr, nullptr);
        }
    }

    void RenderDoc::end_capture()
    {
        if (g_Rdoc_Api) {
            g_Rdoc_Api->EndFrameCapture(nullptr, nullptr);
        }
    }
}
