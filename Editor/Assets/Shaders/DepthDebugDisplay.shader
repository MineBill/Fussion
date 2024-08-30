#version 450 core
#pragma depth_clamp: false

#include "Global.glsl"
#include "Scene.glsl"
#include "Lighting.glsl"

layout (push_constant) uniform PushConstants {
    int CascadeIndex;
} u_PerObjectData;

struct VertexOutput {
    vec2 UV;
};

#pragma type: vertex

const vec4 plane[6] = vec4[](
    vec4(-1.0,  1.0,  0.0, 1.0),
    vec4(-1.0, -1.0,  0.0, 0.0),
    vec4( 1.0, -1.0,  1.0, 0.0),

    vec4(-1.0,  1.0,  0.0, 1.0),
    vec4( 1.0, -1.0,  1.0, 0.0),
    vec4( 1.0,  1.0,  1.0, 1.0)
);

layout(location = 0) out VertexOutput Out;
void Vertex()
{
    Out.UV = plane[gl_VertexIndex].zw;
    vec2 p = plane[gl_VertexIndex].xy;
    gl_Position = vec4(p, 0.0, 1.0);
}

#pragma type: fragment

#define SCENE_SET 1
layout(set = SCENE_SET,  binding = 2) uniform sampler2DArray uShadowMapArray;

layout (location = 0) out vec4 o_Color;

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Convert depth from [0,1] to [-1,1]
    return (2.0 * near * far) / (far + near - z * (far - near)); // Linearize depth
}

layout (location = 0) in VertexOutput In;
void Fragment()
{
    float depth = texture(uShadowMapArray, vec3(In.UV, u_PerObjectData.CascadeIndex)).r;
//    float z = LinearizeDepth(gl_FragCoord.z, 0.1, 500.0);
//    o_Color = vec4(z, z, z, 1.0);
    o_Color = vec4(depth, depth, depth, 1.0);
//    o_Color = vec4(1, 0.2, 0.4, 1.0);
}
