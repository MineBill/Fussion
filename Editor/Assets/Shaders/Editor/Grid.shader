#version 450 core

#pragma blending: on
#pragma samples: 8

#include "Global.glsl"

struct VertexOutput {
    vec3 near_point;
    vec3 far_point;
    mat4 projection;
    mat4 view;
};

#pragma type: vertex

layout (location = 0) out VertexOutput Out;

vec3 plane[6] = vec3[](
    vec3(-1, -1, 0), vec3(1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, 1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

vec3 unproject_point(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void Vertex() {
    vec3 p = plane[gl_VertexIndex].xyz;

    Out.projection = u_ViewData.projection;
    Out.view = u_ViewData.view;

    Out.near_point = unproject_point(p.x, p.y, -1.0, u_ViewData.view, u_ViewData.projection).xyz;
    Out.far_point = unproject_point(p.x, p.y, 1.0, u_ViewData.view, u_ViewData.projection).xyz;

    gl_Position = vec4(p, 1.0);
}

#pragma type: fragment

layout (location = 0) out vec4 o_Color;
layout (location = 0) in VertexOutput In;

vec4 grid(vec3 frag_pos, float scale) {
    vec2 coord = frag_pos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;

    float line = min(grid.x, grid.y);
    float minz = min(derivative.y, 1);
    float minx = min(derivative.x, 1);

    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    if (frag_pos.x > -0.5 * minx && frag_pos.x < 0.5 * minx)
    color.xyz = vec3(3, 51, 202) / 255.0;

    if (frag_pos.z > -0.5 * minz && frag_pos.z < 0.5 * minz)
    color.xyz = vec3(237, 35, 35) / 255.0;

    return color;
}

// TODO(minebill): These need to be parameters.
const float near = 0.1;
const float far = 1000;

float compute_depth(vec3 pos) {
    vec4 clip_space_pos = In.projection * In.view * vec4(pos.xyz, 1.0);
    float t = (clip_space_pos.z / clip_space_pos.w);
    return (t);
}

float compute_linear_depth(vec3 pos) {
    vec4 clip_space_pos = In.projection * In.view * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w);
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far;
}

void Fragment() {
    float t = (0 - In.near_point.y) / (In.far_point.y - In.near_point.y);

    vec3 frag_pos = In.near_point + (t) * (In.far_point - In.near_point);

    float d = compute_depth(frag_pos);
    gl_FragDepth = d;

    float linear_depth = compute_linear_depth(frag_pos);
    float fading = max(0, (0.5 - linear_depth));

    o_Color = (grid(frag_pos, 1) + grid(frag_pos, 0.1)) * float(t > 0);
    o_Color.a *= fading;
}
// vim:ft=glsl
