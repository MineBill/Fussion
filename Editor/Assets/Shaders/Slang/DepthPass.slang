import Common;

struct InstanceData {
    float4x4 Model;
    float4x4 LightSpace;
}

[[vk::binding(0, 0)]]
StructuredBuffer<InstanceData> uInstanceData;

[Shader("vertex")]
float4 VS_Main(SurfaceGeometry surface, uint idx: SV_InstanceID)
    : SV_Position
{
    return uInstanceData[idx].LightSpace * uInstanceData[idx].Model * float4(surface.Position, 1);
}


