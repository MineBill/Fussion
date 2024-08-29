#include "FussionPCH.h"
#include "RenderTypes.h"

namespace Fussion {

    void RenderContext::AddRenderObject(RenderObject& obj)
    {
        RenderObjects.push_back(obj);
    }

    void RenderContext::Reset()
    {
        RenderObjects.clear();
    }
}
