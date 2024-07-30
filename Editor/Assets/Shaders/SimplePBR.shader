#version 450 core

#include "Global.glsl"
#include "Lighting.glsl"

layout (std140, set = 2, binding = 0) uniform Material {
    vec4 albedo_color;
} u_Material;

layout (push_constant) uniform PushConstants {
    mat4 Model;
} u_PushConstants;

struct VertexOutput {
    vec3 Normal;
    vec2 UV;
};

#pragma type: vertex

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
//layout(location = 2) in vec3 a_Tangent;
layout (location = 2) in vec2 a_UV;
//layout(location = 4) in vec3 a_Color;

layout (location = 0) out VertexOutput Out;

void Vertex() {
    Out.Normal = a_Normal;
    Out.UV = a_UV;
    gl_Position =
    u_ViewData.projection *
    u_ViewData.view *
    u_PushConstants.Model *
    vec4(a_Position, 1.0);
}

#pragma type: fragment

layout (location = 0) out vec4 o_Color;
layout (location = 0) in VertexOutput In;

void Fragment() {
    o_Color = u_Material.albedo_color;
}
