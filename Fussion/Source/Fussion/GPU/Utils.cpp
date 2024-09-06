#include "Utils.h"

#include "C:/Users/Jonathan/scoop/apps/renderdoc/1.34/renderdoc_app.h"
#include "FussionPCH.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace Fussion::GPU::Utils {
    RENDERDOC_API_1_1_2* rdoc_api = NULL;

    void RenderDoc::Initialize()
    {
        if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
            assert(ret == 1);

            LOG_INFO("RenderDoc connection active");
        } else {
            LOG_INFO("No RenderDoc connection.");
        }
    }

    void RenderDoc::StartCapture()
    {
        if (rdoc_api) {
            rdoc_api->StartFrameCapture(nullptr, nullptr);
        }
    }

    void RenderDoc::EndCapture()
    {
        if (rdoc_api) {
            rdoc_api->EndFrameCapture(nullptr, nullptr);
        }
    }
}
