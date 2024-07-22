#version 450

layout (set = 2, binding = 0) uniform sampler2DArray s_ShadowMap;

struct VertexOutput {
    vec3 frag_color;
    vec2 frag_uv;
    vec3 frag_pos;
};

#pragma type: vertex

layout (location = 0) out VertexOutput Out;

vec3 plane[6] = vec3[](
    vec3(-1, -1, 0), vec3(1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, 1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

vec2 uv[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

void Vertex() {
    Out.frag_color = vec3(1.0, 1.0, 1.0);
    Out.frag_uv = uv[gl_VertexIndex];
    gl_Position = vec4(plane[gl_VertexIndex], 1.0);
}

#pragma type: fragment

layout (location = 0) out vec4 o_Color;

layout (location = 0) in VertexOutput In;

void Fragment() {
    o_Color = texture(s_ShadowMap, vec3(In.frag_uv, 2));
}