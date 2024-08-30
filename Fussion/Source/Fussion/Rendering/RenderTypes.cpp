#include "FussionPCH.h"
#include "RenderTypes.h"

namespace Fussion {

    void RenderContext::AddRenderObject(RenderObject& obj)
    {
        size_t index = RenderObjects.size();
        RenderObjects.push_back(obj);

        MeshRenderLists[obj.VertexBuffer].push_back(index);
    }

    void RenderContext::Reset()
    {
        RenderObjects.clear();

        for (auto& list : MeshRenderLists) {
            list.second.clear();
        }
        MeshRenderLists.clear();
    }
}
