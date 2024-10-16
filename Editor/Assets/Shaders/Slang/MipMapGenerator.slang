import Common;

struct VertexOutput {
    float4 Position : SV_Position;
    float2 UV;
}

[[vk::binding(0, 0)]]
Texture2D uTexture;
[[vk::binding(1, 0)]]
SamplerState uSampler;

static const float3[6] plane = {
    float3(-1, -1, 0), float3(1, -1, 0), float3(-1, 1, 0),
    float3(-1, 1, 0), float3(1, -1, 0), float3(1, 1, 0)
};

static const float2[6] uvs = {
    float2(0, 1), float2(1, 1), float2(0, 0),
    float2(0, 0), float2(1, 1), float2(1, 0)
};

[Shader("vertex")]
VertexOutput VS_Main(uint idx: SV_VertexID)
{
    VertexOutput out;
    out.Position = float4(plane[idx], 1);
    out.UV = uvs[idx];
    return out;
}

[Shader("fragment")]
float4 FS_Main(VertexOutput in)
    : SV_Target
{
    return uTexture.Sample(uSampler, in.UV);
}


