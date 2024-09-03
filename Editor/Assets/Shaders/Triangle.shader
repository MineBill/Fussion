#version 450 core

layout(std140, set = 0, binding = 0) uniform ViewData {
    mat4 projection;
    mat4 view;
} view_data;

#pragma type: vertex

void Vertex() {
    vec2[] pos = vec2[](
        vec2( 0.0,  0.5),
        vec2(-0.5, -0.5),
        vec2( 0.5, -0.5)
    );
    vec2 p = pos[gl_VertexIndex].xy;

    gl_Position = view_data.projection * view_data.view * vec4(p, 0, 1);
}

#pragma type: fragment

layout(location = 0) out vec4 color0;

void Fragment() {
    color0 = vec4(1, sin(1.0 / 128.0), 0, 1);
}