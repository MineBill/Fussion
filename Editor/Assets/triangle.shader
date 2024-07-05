#version 450 core

#include "global.glsl"

//layout(set = 2, binding = 1) uniform sampler2D u_Texture;

struct VertexOutput {
    vec3 Color;
    vec2 UV;
};

#pragma type: vertex

const vec3 triangle[] = vec3[](
    vec3( 0.0,  0.5, 0.0),
    vec3( 0.5, -0.5, 0.0),
    vec3(-0.5, -0.5, 0.0),

    vec3(-0.5, -0.5, 0.0),
    vec3( 0.5, -0.5, 0.0),
    vec3( 0.0,  0.5, 0.0)
);

const vec2 uvs[] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),

    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

const vec3 colors[] = vec3[](
    vec3( 1.0,  0.0, 0.0),
    vec3( 0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),

    vec3(0.0, 1.0, 1.0),
    vec3( 0.0, 1.0, 0.0),
    vec3( 1.0,  0.0, 1.0)
);

layout (location = 0) out VertexOutput Out;

void main() {
    // vertex code
    Out.Color = colors[gl_VertexIndex];
    Out.UV = uvs[gl_VertexIndex];
    gl_Position = u_ViewData.projection * u_ViewData.view * vec4(triangle[gl_VertexIndex], 1.0);
}

#pragma type: fragment

layout(location = 0) out vec4 o_Color;
layout(location = 0) in VertexOutput In;

void main() {
    o_Color = vec4(In.Color, 1.0);
}
