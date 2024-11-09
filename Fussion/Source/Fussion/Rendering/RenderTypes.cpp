#include "FussionPCH.h"
#include "RenderTypes.h"

#include <tracy/Tracy.hpp>

namespace Fussion {

    void RenderContext::add_render_object(RenderObject const& obj)
    {
        ZoneScoped;
        size_t index = render_objects.size();
        render_objects.push_back(obj);

        mesh_render_lists[obj.material][obj.vertex_buffer.Handle].push_back(index);
    }

    void RenderContext::reset()
    {
        render_objects.clear();
        directional_lights.clear();

        for (auto& map : mesh_render_lists) {
            for (auto& list : map.second) {
                list.second.clear();
            }
        }

        post_processing_settings.use_ssao = false;
        environment_texture = nullptr;
    }
}
