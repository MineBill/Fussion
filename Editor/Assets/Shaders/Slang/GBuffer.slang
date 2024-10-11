import common;
#include "Include/GlobalUniforms.slang"

struct VertexOutput {
    float4 Position : SV_Position;
    float3 Pos;
    float3 Normal;
}

struct InstanceData {
    float4x4 Model;
}

[[vk::binding(0, 1)]]
StructuredBuffer<InstanceData> uInstanceData;

[shader("vertex")]
VertexOutput VS_Main(SurfaceGeometry surface, uint idx: SV_InstanceID)
{
    VertexOutput o;
    float4x4 model = uInstanceData[idx].Model;
    float4 worldPos = model * float4(surface.Position, 1.0);
    // o.Position = mul(mul(uViewData.Projection, uViewData.View), worldPos);
    o.Position = (uViewData.Projection * uViewData.View) * worldPos;

    float4x4 modelInverse = inverse(model);
    float3x3 normalMatrix = transpose(float3x3(modelInverse));
    o.Normal = normalize(mul(normalMatrix, surface.Normal));
    o.Pos = (uViewData.View * worldPos).xyz;
    return o;
}

struct FragmentOutput {
    float4 Position : SV_Target;
    float4 Normal : SV_Target1;
    float4 Albedo : SV_Target2;
}

[shader("fragment")]
FragmentOutput FS_Main(VertexOutput i)
{
    FragmentOutput o;
    o.Position = float4(i.Pos.xyz, 1.0);
    // o.Position.xy /= uViewData.ScreenSize;
    o.Normal = float4(normalize(i.Normal.rgb), 1.0);
    o.Albedo = float4(0, 0, 1, 1);
    return o;
}


