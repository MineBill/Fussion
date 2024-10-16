import Common;
import glsl;
#include "Include/GlobalUniforms.slang"

struct VSOutput {
    float4 Position : SV_Position;

    float3 NearPoint;
    float3 FarPoint;
    float4x4 Projection;
    float4x4 View;
}

static const float3 plane[6] = {
    float3(-1, -1, 0), float3(1, -1, 0), float3(-1, 1, 0),
    float3(-1, 1, 0), float3(1, -1, 0), float3(1, 1, 0)
};

float3 UnprojectPoint(float x, float y, float z, float4x4 view, float4x4 projection)
{
    let viewInv = inverse(view);
    let projInv = inverse(projection);
    let unprojected = mul(mul(viewInv, projInv), float4(x, y, z, 1.0));
    return unprojected.xyz / unprojected.w;
}

[[shader("vertex")]]
VSOutput VS_Main(uint idx: SV_VertexID)
{
    VSOutput output;

    float3 p = plane[idx];

    output.Projection = uViewData.Projection;
    output.View = uViewData.View;

    output.NearPoint = UnprojectPoint(p.x, p.y, -1.0, uViewData.View, uViewData.Projection);
    output.FarPoint = UnprojectPoint(p.x, p.y, 1.0, uViewData.View, uViewData.Projection);

    output.Position = float4(p, 1.0);
    return output;
}

float4 Grid(float3 fragPos, float scale)
{
    const vec3 Z_AXIS_COLOR = vec3(3, 51, 202) / 255.0;
    const vec3 X_AXIS_COLOR = vec3(237, 35, 35) / 255.0;
    const vec3 BASE_GRID_COLOR = vec3(0.1, 0.1, 0.1);

    vec2 coord = fragPos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;

    float line = min(grid.x, grid.y);
    float minz = min(derivative.y, 1);
    float minx = min(derivative.x, 1);

    vec4 color = vec4(BASE_GRID_COLOR, 1.0 - min(line, 1.0));

    if (fragPos.x > -0.5 * minx && fragPos.x < 0.5 * minx) {
        color.xyz = Z_AXIS_COLOR;
    }

    if (fragPos.z > -0.5 * minz && fragPos.z < 0.5 * minz) {
        color.xyz = X_AXIS_COLOR;
    }

    return color;
}

static const float Near = 0.1;
static const float Far = 1000;

float ComputeDepth(float4x4 projection, float4x4 view, float3 pos)
{
    let clipSpacePos = mul(mul(projection, view), float4(pos.xyz, 1.0));
    return clipSpacePos.z / clipSpacePos.w;
}

float ComputeLinearDepth(float4x4 projection, float4x4 view, float3 pos)
{
    let clipSpacePos = mul(mul(projection, view), float4(pos.xyz, 1.0));
    float clipSpaceDepth = (clipSpacePos.z / clipSpacePos.w);
    float linearDepth = (2.0 * Near * Far) / (Far + Near - clipSpaceDepth * (Far - Near)); // get linear value between 0.01 and 100
    return linearDepth / Far;
}

struct FragmentOutput {
    float4 Color : SV_Target;
    float Depth : SV_Depth;
}

[[shader("fragment")]]
FragmentOutput FS_Main(VSOutput input)
{
    FragmentOutput output;
    float t = (0 - input.NearPoint.y) / (input.FarPoint.y - input.NearPoint.y);
    float3 fragPos = input.NearPoint + (t) * (input.FarPoint - input.NearPoint);

    float d = ComputeDepth(input.Projection, input.View, fragPos);
    output.Depth = d;

    float linearDepth = ComputeLinearDepth(input.Projection, input.View, fragPos);
    float fading = max(0, (0.5 - linearDepth));

    output.Color = (Grid(fragPos, 1) + Grid(fragPos, 0.1)) * float(t > 0);
    output.Color.a *= fading;
    return output;
}


