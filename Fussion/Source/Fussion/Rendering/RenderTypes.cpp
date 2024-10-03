#include "FussionPCH.h"
#include "RenderTypes.h"

#include <tracy/Tracy.hpp>

namespace Fussion {

    void RenderContext::AddRenderObject(RenderObject const& obj)
    {
        ZoneScoped;
        size_t index = RenderObjects.size();
        RenderObjects.push_back(obj);

        MeshRenderLists[obj.Material][obj.VertexBuffer.Handle].push_back(index);
    }

    void RenderContext::Reset()
    {
        RenderObjects.clear();
        DirectionalLights.clear();

        for (auto& map : MeshRenderLists) {
            for (auto& list : map.second) {
                list.second.clear();
            }
        }

        PostProcessingSettings.UseSSAO = false;
        EnvironmentMap = nullptr;
    }
}
