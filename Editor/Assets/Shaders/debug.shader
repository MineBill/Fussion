#version 450

#pragma topology: lines
#pragma samples: 8

#include "Global.glsl"

struct VertexOutput {
    vec4 Color;
    float Thickness;
};

#pragma type: vertex

layout (location = 0) in vec3 Position;
layout (location = 1) in float Thickness;
layout (location = 2) in vec4 Color;

layout (location = 0) out VertexOutput Out;
void Vertex() {
    Out.Color = Color;
    Out.Thickness = Thickness;
    gl_Position = u_ViewData.projection * u_ViewData.view * vec4(Position, 1.0);
}

#pragma type: fragment

layout (location = 0) out vec4 o_Color;

layout (location = 0) in VertexOutput In;
void Fragment() {
    o_Color = In.Color;
}
// vim:ft=glsl
