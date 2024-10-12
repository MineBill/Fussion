import common;
#include "Include/GlobalUniforms.slang"
#include "Include/SceneUniforms.slang"

struct VertexOutput {
    float4 Position : SV_Position;
    float3 Pos;
    float3 FSun;
}

static const float2[4] Positions = {
    float2(-1, 1),
    float2(-1, -1),
    float2(1, 1),
    float2(1, -1),
};

[Shader("vertex")]
VertexOutput VS_Main(uint idx: SV_VertexID)
{
    VertexOutput out;
    out.Position = float4(Positions[idx], 0, 1);
    let www = transpose(float3x3(uViewData.View));
    out.Pos = mul(www, (inverse(uViewData.Projection) * out.Position).xyz);
    out.FSun = float3(0, 0.05, 0);
    return out;
}

float Mod289(float x)
{
    return float(x - floor(x * (1.0 / 289.0)) * 289.0);
}

float4 Mod289_V(float4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 Perm(float4 x)
{
    return Mod289_V(((x * 34.0) + 1.0) * x);
}

float Noise(float3 p)
{
    let a = floor(p);
    var d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    let b = a.xxyy + float4(0.0, 1.0, 0.0, 1.0);
    let k1 = Perm(b.xyxy);
    let k2 = Perm(k1.xyxy + b.xyxy);

    let c = k2 + a.zzzz;
    let k3 = Perm(c);
    let k4 = Perm(c + 1.0);

    let o1 = fract(k3 * (1.0 / 41.0));
    let o2 = fract(k4 * (1.0 / 41.0));

    let o3 = o2 * d.z + o1 * (1.0 - d.z);
    let o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

static const float3x3 m = { 0.0, 1.6, 1.2, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28 };

float FBM(float3 pp)
{
    var p = pp;
    var f = 0.0;
    f += Noise(p) / 2.0;
    p = mul(m, p * 1.1);
    f += Noise(p) / 4.0;
    p = mul(m, p * 1.2);
    f += Noise(p) / 6.0;
    p = mul(m, p * 1.3);
    f += Noise(p) / 12.0;
    p = mul(m, p * 1.4);
    f += Noise(p) / 24.0;
    return f;
}

struct GlobalData {
    float Time;
}

struct FragmentOutput {
    float4 Color : SV_Target;
    float Depth : SV_Depth;
}

[Shader("fragment")]
FragmentOutput FS_Main(VertexOutput in) {
    if (in.Pos.y < 0) {
        discard;
    }
    var out : FragmentOutput;
    var globalData : GlobalData;
    globalData.Time = 1.0;

    let Br = 0.0025;
    let Bm = 0.0003;
    let g = 0.98;

    let nitrogen = float3(0.650, 0.570, 0.475);
    let Kr = Br / pow(nitrogen, float3(4.0));
    let Km = Bm / pow(nitrogen, float3(0.84));

    let mu = dot(normalize(in.Pos), normalize(uLightData.Directional.Direction.xyz));
    let rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    let mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    let day_extinction = exp(-exp(-((in.Pos.y + uLightData.Directional.Direction.xyz.y * 4.0) * (exp(-in.Pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-in.Pos.y * 16.0) + 0.1) * Kr / Br) * exp(-in.Pos.y * exp(-in.Pos.y * 8.0) * 4.0) * exp(-in.Pos.y * 2.0) * 4.0;
    let night_extinction = float3(1.0 - exp(uLightData.Directional.Direction.y)) * 0.2;
    let extinction = lerp(day_extinction, night_extinction, -uLightData.Directional.Direction.y * 0.2 + 0.5);
    var color = float4(rayleigh * mie * extinction, 1.0).xyz;

    let cirrus = 0.5;
    // Cirrus Clouds
    var density = smoothstep(1.0 - cirrus, 1.0, FBM(in.Pos.xyz / in.Pos.y * 1.0 + globalData.Time * 0.01)) * 0.3;
    color = float4(lerp(color.rgb, extinction * 10.0, density * max(in.Pos.y, 0.0)), 1.0).xyz;

    density = smoothstep(1.0 - cirrus, 1.0, FBM(in.Pos.xyz / in.Pos.y * 4.0 + globalData.Time * 0.01)) * 0.3;
    color = float4(lerp(color.rgb, extinction * 10.0, density * max(in.Pos.y, 0.0)), 1.0).xyz;
    // color = pow(1.0 - exp(-1.3 * color.rgb), float3(1.3));
    out.Color = float4(color, 1.0);

    out.Depth = 1.0;
    return out;
}


