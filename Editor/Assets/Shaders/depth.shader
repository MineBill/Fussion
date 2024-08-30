#version 450 core
#include "Defines.glsl"

#pragma depth_clamp: true

#pragma type: vertex

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec2 a_UV;
layout (location = 4) in vec3 a_Color;

struct InstanceData {
    mat4 Model;
    mat4 LightSpace;
};

layout (std140, set = 0, binding = 0) readonly buffer InstanceBuffer {
    InstanceData Data[];
} b_InstanceBuffer;

void Vertex()
{
    gl_Position = b_InstanceBuffer.Data[gl_InstanceIndex].LightSpace * b_InstanceBuffer.Data[gl_InstanceIndex].Model * vec4(a_Position, 1.0);
}

#pragma type: fragment

void Fragment()
{
}
