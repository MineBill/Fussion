#ifndef GLOBAL_SET_H
#define GLOBAL_SET_H

#include "Defines.glsl"

layout (std140, set = GLOBAL_SET, binding = 0) uniform ViewData {
    mat4 projection;
    mat4 view;
    mat4 rotation_view;
    vec2 screen_size;
} u_ViewData;

layout (std140, set = GLOBAL_SET, binding = 1) uniform DebugOptions {
    int shadow_cascade_boxes;
    int shadow_cascade_colors;
} u_DebugOptions;

layout (std140, set = GLOBAL_SET, binding = 2) uniform GlobalData {
    float time;
} u_GlobalData;


#endif
