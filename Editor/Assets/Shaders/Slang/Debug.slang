import Common;
#include "Include/GlobalUniforms.slang"

struct VertexInput {
    float3 Position;
    float Thickness;
    float4 Color;
}

struct VertexOutput {
    float4 Position : SV_Position;
    float4 Color;
    float Thickness;
}

[shader("vertex")]
VertexOutput VS_Main(VertexInput input)
{
    VertexOutput output;
    output.Color = input.Color;
    output.Thickness = input.Thickness;
    output.Position = mul(mul(uViewData.Projection, uViewData.View), float4(input.Position, 1.0));
    return output;
}

[shader("fragment")]
float4 FS_Main(VertexOutput input)
    : SV_Target
{
    return input.Color;
}


