import Common;
import Lights;
import Math;
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
[[vk::binding(1, 2)]]
ConstantBuffer<PbrMaterial> uMaterial;
[[vk::binding(2, 2)]]
Texture2D uAlbedo;
[[vk::binding(3, 2)]]
Texture2D uNormal;
[[vk::binding(4, 2)]]
Texture2D uMetallicRoughness;
[[vk::binding(5, 2)]]
Texture2D uOcclusion;
[[vk::binding(6, 2)]]
Texture2D uEmissive;
[[vk::binding(7, 2)]]
SamplerState uLinearSampler;
[[vk::binding(8, 2)]]
SamplerComparisonState uShadowSampler;

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

float SampleShadow(uint index, float2 coords, float compare, float2 texelSize)
{
    float result = 0;
    static const float samples = 16;
    for (uint i = 0; i < samples; ++i) {
        result += uShadowTex.SampleCmp(uShadowSampler, float3(coords + SamplePoisson(i) * texelSize * 2.0f, float(index)), compare);
    }
    return result / float(samples);
}

float DoDirectionalShadow(VertexOutput in, DirectionalLight light, uint index)
{
    float shadow = 0.0;
    uint i = 0;

    float4 fragPosLightSpace = in.PosLightSpace[0];
    if (index == 1) {
        fragPosLightSpace = in.PosLightSpace[1];
    } else if (index == 2) {
        fragPosLightSpace = in.PosLightSpace[2];
    } else if (index == 3) {
        fragPosLightSpace = in.PosLightSpace[3];
    }
    float3 coords = (fragPosLightSpace.xyz / fragPosLightSpace.w);
    if (coords.z > 1.0 || coords.z < -1.0) {
        return 1.0;
    }
    if (coords.y > 1.0 || coords.y < -1.0) {
        return 1.0;
    }
    if (coords.x > 1.0 || coords.x < -1.0) {
        return 1.0;
    }
    coords.y = 1.0 - coords.y;
    uint2 dim;
    uint numLevels;
    uint elements;
    uShadowTex.GetDimensions(0, dim.x, dim.y, elements, numLevels);

    float bias = max((1.0 / 4096.0) * (1.0 - dot(in.Normal, normalize(light.Direction.xyz))), 0.003);
    float2 texelSize = float2(1.0, 1.0) / float2(dim);

    return SampleShadow(index, coords.xy, coords.z - bias, texelSize);
}

float3 DoDirectionalLight(DirectionalLight light, VertexOutput in)
{
    float3 n = uNormal.Sample(uLinearSampler, in.FragUV * uMaterial.TilingFactor).rgb;
    n = normalize(n * 2 - 1);

    float3 tangentViewPos = mul(in.TBN, uViewData.Position.xyz);
    float3 tangentFragPos = mul(in.TBN, in.FragPos.xyz);
    float3 tangentLightDir = mul(in.TBN, light.Direction.xyz);

    float3 v = normalize(tangentViewPos - tangentFragPos);
    float3 l = normalize(tangentLightDir);
    float3 h = normalize(v + l);

    float3 radiance = light.Color.rgb * light.Brightness;
    float3 albedo = uAlbedo.Sample(uLinearSampler, in.FragUV * uMaterial.TilingFactor).rgb * uMaterial.AlbedoColor.rgb;
    float4 metallicRoughness = uMetallicRoughness.Sample(uLinearSampler, in.FragUV * uMaterial.TilingFactor);
    float metalness = metallicRoughness.b * uMaterial.MetallicFactor;
    float3 f0 = lerp(float3(0.04), albedo, metalness);

    float roughness = metallicRoughness.g * uMaterial.RoughnessFactor;
    float ndf = PBR::DistributionGGX(n, h, roughness);
    float g = PBR::GeometrySmith(n, v, l, roughness);

    float3 f = PBR::FresnelSchlick(max(dot(h, v), 0.0), f0, roughness);
    float3 numerator = ndf * g * f;
    float3 denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
    float3 specular = numerator / denominator;

    float3 ks = f;
    float3 kd = float3(1) - ks;
    kd *= 1 - metalness;

    float3 i = in.FragPos.xyz - uViewData.Position.xyz;
    float3 r = reflect(i, normalize(in.Normal));
    float3 reflection = float3(1, 1, 1);

    float3 ndotl = max(dot(n, l), 0);

    uint index = 0;
    for (uint i = 0; i < 4; ++i) {
        if (in.ViewPos.z < uLightData.ShadowSplitDistances[i]) {
            index = i + 1;
        }
    }

    float shadow = clamp(DoDirectionalShadow(in, light, index), 0.0, 1.0);

    float occlusion = uOcclusion.Sample(uLinearSampler, in.FragUV * uMaterial.TilingFactor).r;
    float ssaoOcclusion = uSSAOTex.Sample(uLinearSampler, in.Position.xy / uViewData.ScreenSize).r;

    float3 irradiance = uEnvironmentTex.Sample(uLinearSampler, n).rgb;
    float3 diffuse = irradiance * albedo;
    float3 ambient = (kd * diffuse) * occlusion * ssaoOcclusion;

    float3 ret = ambient + (kd * albedo / PI + specular) * radiance * ndotl * shadow;
    ret += uEmissive.Sample(uLinearSampler, in.FragUV * uMaterial.TilingFactor).rgb;
    return ret;
}

[Shader("fragment")]
float4 FS_Main(VertexOutput in)
    : SV_Target
{
    float3 Lo = float3(0);
    Lo += DoDirectionalLight(uLightData.Directional, in);
    return float4(Lo, 1);
}


