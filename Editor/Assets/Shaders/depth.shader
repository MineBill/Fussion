#version 450 core
#pragma depth_clamp: true

layout (push_constant) uniform DepthPassPushConstants {
    mat4 model;
    mat4 light_space;
} u_PerObjectData;

#pragma type: vertex

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Tangent;
layout(location = 3) in vec2 a_UV;
layout(location = 4) in vec3 a_Color;

void Vertex()
{
    gl_Position = u_PerObjectData.light_space * u_PerObjectData.model * vec4(a_Position, 1.0);
}

#pragma type: fragment

void Fragment()
{
}
