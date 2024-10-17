module Math;

public float2 SampleSpherical(float3 v)
{
    let inv_atan = float2(0.1591, 0.3183);
    var uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

public float3 Reinhard(float3 x)
{
    return x / (1.0 + x);
}


