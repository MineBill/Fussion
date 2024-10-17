import Common;
import Math;

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
TextureCube uCubeTexture;
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

[Shader("fragment")]
float4 FS_Main(VertexOutput in)
    : SV_Target
{
    let normal = normalize(in.FragPosition);
    var irradiance = float3(0.0);

    var up = float3(0, 1, 0);
    let right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    let PI = 3.14159265359;
    let sample_delta = 0.025;
    var nr_samples = 0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sample_delta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sample_delta) {
            let tangent_sample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            let sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += uCubeTexture.Sample(uSampler, normalize(sample_vec)).rgb * cos(theta) * sin(theta);
            nr_samples += 1;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nr_samples));
    return float4(irradiance, 1.0);
}


