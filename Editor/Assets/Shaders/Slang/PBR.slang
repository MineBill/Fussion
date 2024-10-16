import Common;
#include "Include/GlobalUniforms.slang"
#include "Include/SceneUniforms.slang"

static const float4x4 ShadowBias = {
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
};

struct VertexOutput {
    float4 Position : SV_Position;

    float3 FragColor;
    float2 FragUV;
    float4 FragPos;

    float3 Normal;
    float4 ViewPos;

    float3x3 TBN;
    float4[4] PosLightSpace;
}

struct InstanceData {
    float4x4 Model;
}

[[vk::binding(0, 2)]]
StructuredBuffer<InstanceData> uInstanceData;

[Shader("vertex")]
VertexOutput VS_Main(SurfaceGeometry surface, uint idx: SV_InstanceID)
{
    VertexOutput out;

    float4x4 model = uInstanceData[idx].Model;

    float3x3 normal = transpose(float3x3(inverse(model)));
    float3 t = normalize(mul(normal, surface.Tangent.xyz));
    float3 n = normalize(mul(normal, surface.Normal));
    t = normalize(t - dot(t, n) * n);
    float3 b = cross(n, t);
    out.TBN = float3x3(t, b, n);

    out.FragPos = model * float4(surface.Position, 1);
    out.ViewPos = uViewData.View * out.FragPos;

    out.FragColor = surface.Color;
    out.FragUV = surface.UV;
    out.Position = uViewData.Projection * uViewData.View * model * float4(surface.Position, 1);

    for (uint i = 0; i < 4; ++i) {
        out.PosLightSpace[i] = ShadowBias * uLightData.Directional.LightSpaceMatrix[i] * out.FragPos;
    }
    return out;
}

[Shader("fragment")]
float4 FS_Main(VertexOutput in) : SV_Target {
    float3 Lo;

    return float4(Lo, 1);
}
