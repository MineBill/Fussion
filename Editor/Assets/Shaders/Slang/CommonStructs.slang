implementing Common;

public struct ViewData {
    public float4x4 Projection;
    public float4x4 View;
    public float4x4 RotationView;
    public float4 Position;
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
    public float Brightness;
}

public struct LightData {
    public DirectionalLight Directional;
    public float4 ShadowSplitDistances;
}

public struct PbrMaterial {
    public float4 AlbedoColor;
    public float MetallicFactor;
    public float RoughnessFactor;
    public float2 TilingFactor;
}


