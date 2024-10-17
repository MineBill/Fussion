import Common;

struct VertexOutput {
    float4 Position : SV_Position;
    float3 FragPosition;
}

struct LocalSurface {
    float3 Position;
}

struct PerFaceViewData {
    float4x4 View;
}

[[vk::binding(0, 0)]]
ConstantBuffer<PerFaceViewData> uViewData;
[[vk::binding(1, 0)]]
Texture2D uHDRTexture;
[[vk::binding(2, 0)]]
SamplerState uSampler;

[Shader("vertex")]
VertexOutput VS_Main(LocalSurface surface, uint idx: SV_VertexID)
{
    VertexOutput out;
    out.FragPosition = surface.Position;
    out.Position = uViewData.View * float4(surface.Position, 1.0);
    return out;
}

float2 SampleSpherical(float3 v)
{
    let inv_atan = float2(0.1591, 0.3183);
    var uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

float3 Reinhard(float3 x)
{
    return x / (1.0 + x);
}

[Shader("fragment")]
float4 FS_Main(VertexOutput in)
    : SV_Target
{
    float2 uv = SampleSpherical(normalize(in.FragPosition));

    float3 sdr = Reinhard(uHDRTexture.Sample(uSampler, uv).rgb);
    return float4(sdr, 1.0);
}


