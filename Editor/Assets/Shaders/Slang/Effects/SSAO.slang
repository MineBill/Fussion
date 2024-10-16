import Common;
#include "Include/GlobalUniforms.slang"

static const int g_KernelSize = 64;

struct Data {
    float3 Sample;
}

struct Options {
    uint KernelSize;
    float Radius;
    float Bias;
    float NoiseScale;
}

[[vk::binding(0, 1)]]
Texture2D uPositionTex;
[[vk::binding(1, 1)]]
Texture2D uNormalTex;
[[vk::binding(2, 1)]]
Texture2D uNoiseTex;
[[vk::binding(3, 1)]]
SamplerState uSampler;
[[vk::binding(4, 1)]]
SamplerState uNoiseSampler;

[[vk::binding(5, 1)]]
StructuredBuffer<Data> uData;
[[vk::binding(6, 1)]]
ConstantBuffer<Options> uOptions;

struct VertexOutput {
    float4 Position : SV_Position;
    float2 UV;
}

[shader("vertex")]
VertexOutput VS_Main(uint idx: SV_VertexID)
{
    VertexOutput o;
    float x = float(idx & 2);
    float y = float(idx & 1);
    o.Position = float4(2, 4, 0, 1) * float4(x, y, 0, 1) - float4(1, 1, 0, 0);
    o.UV = float2(1.0, 2.0) * float2(x, -y);
    return o;
}

[shader("fragment")]
float FS_Main(VertexOutput i)
    : SV_Target
{
    float3 fragPos = uPositionTex.Sample(uSampler, i.UV).xyz;
    float3 fragNormal = uNormalTex.Sample(uSampler, i.UV).rgb;

    float2 noiseScale = uViewData.ScreenSize / uOptions.NoiseScale;
    float3 randomVec = uNoiseTex.Sample(uNoiseSampler, i.UV * noiseScale).xyz;

    float3 tangent = normalize(randomVec - fragNormal * dot(randomVec, fragNormal));
    float3 bitangent = cross(fragNormal, tangent);
    float3x3 tbn = float3x3(tangent, bitangent, fragNormal);

    float occlusion = 0;
    for (uint i = 0; i < g_KernelSize; ++i) {
        float3 samplePos = mul(tbn, uData[i].Sample);
        samplePos = fragPos + samplePos * uOptions.Radius;

        float4 offset = float4(samplePos, 1);
        offset = uViewData.Projection * offset;
        offset = float4(offset.xyz / offset.w, 1);
        offset = float4(offset.xyz * 0.5 + 0.5, 1);
        offset.y *= -1;

        float sampleDepth = uPositionTex.Sample(uSampler, offset.xy).z;
        if (sampleDepth >= samplePos.z + uOptions.Bias) {
            float rangeCheck = smoothstep(0.0, 1.0, uOptions.Radius / abs(fragPos.z - sampleDepth));
            occlusion += rangeCheck;
        }
    }
    return 1.0 - (occlusion / float(g_KernelSize));
}


