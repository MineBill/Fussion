#version 450 core

layout (push_constant) uniform DepthPassPushConstants {
    mat4 model;
    mat4 light_space;
} u_PerObjectData;

#pragma type: vertex

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_UV;

void Vertex() {
    gl_Position = u_PerObjectData.light_space * u_PerObjectData.model * vec4(a_Position, 1.0);
}

#pragma type: fragment

layout (location = 0) out vec4 o_Color;

float LinearizeDepth(float depth, float near, float far) {
    // float z = depth * 2.0 - 1.0; // Convert depth from [0,1] to [-1,1]
    float z = depth;
    return (2.0 * near * far) / (far + near - z * (far - near)); // Linearize depth
}

void Fragment() {
    float z = LinearizeDepth(gl_FragCoord.z, 0.1, 1000.0);
    o_Color = vec4(z, z, z, 1.0);
}
// vim:ft=glsl
