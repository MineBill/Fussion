import common;

[[vk::binding(0, 0)]]
Texture2D uTexture;
[[vk::binding(1, 0)]]
SamplerState uSampler;

struct VertexOutput {
    float4 Position : SV_Position;
    float2 UV;
}

[Shader("vertex")]
VertexOutput VS_Main(uint idx: SV_VertexID)
{
    VertexOutput out;
    out.UV = float2(float((idx << 1) & 2), float(idx & 2));
    out.Position = float4(out.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return out;
}

[Shader("fragment")]
float FS_Main(VertexOutput i)
    : SV_Target
{
    float2 texelSize = 1.0 / uTexture.GetDim(0);

    float result = 0.0;
    for (int x = -1; x < 1; x++) {
        for (int y = -1; y < 1; y++) {
            var offset = float2(float(x), float(y)) * texelSize;
            offset.y *= -1.0;

            result += uTexture.Sample(uSampler, i.UV + offset).r;
        }
    }

    return result / (4.0);
}


