#include "FussionPCH.h"
#include "RenderTypes.h"

namespace Fussion {

    void RenderContext::add_render_object(RenderObject& obj)
    {
        size_t index = render_objects.size();
        render_objects.push_back(obj);

        mesh_render_lists[obj.vertex_buffer.handle].push_back(index);
    }

    void RenderContext::reset()
    {
        render_objects.clear();
        directional_lights.clear();

        for (auto& list : mesh_render_lists) {
            list.second.clear();
        }
        mesh_render_lists.clear();
    }
}
