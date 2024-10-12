implementing common;

public struct ViewData {
    public float4x4 Projection;
    public float4x4 View;
    public float4x4 RotationView;
    public float2 ScreenSize;
}

public struct DebugOptions {
    public int DoThis;
    public int DoThat;
}

public struct SurfaceGeometry {
    public float3 Position;
    public float3 Normal;
    public float4 Tangent;
    public float2 UV;
    public float3 Color;
}

public struct DirectionalLight {
    public float4x4 LightSpaceMatrix[4];
    public float4 Direction;
    public float4 Color;
}

public struct LightData {
    public DirectionalLight Directional;
    public float4 ShadowSplitDistances;
}


