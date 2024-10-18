module Lights;
import Common;

namespace PBR {
    public float3 FresnelSchlick(float cos_theta, float3 f0, float roughness)
    {
        return f0 + (max(float3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
    }

    public float DistributionGGX(float3 n, float3 h, float roughness)
    {
        let a = roughness * roughness;
        let a2 = a * a;
        let n_dot_h = max(dot(n, h), 0.0);
        let n_dot_h2 = n_dot_h * n_dot_h;

        let num = a2;
        var denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
        denom = pi * denom * denom;

        return num / denom;
    }

    public float GeometrySchlickGGX(float n_dot_v, float roughness)
    {
        let r = roughness + 1.0;
        let k = (r * r) / 8.0;

        let num = n_dot_v;
        let denom = n_dot_v * (1.0 - k) + k;

        return num / denom;
    }

    public float GeometrySmith(float3 n, float3 v, float3 l, float roughness)
    {
        float3 n_dot_v = max(dot(n, v), 0.0);
        float3 n_dot_l = max(dot(n, l), 0.0);
        float ggx2 = GeometrySchlickGGX(n_dot_v, roughness);
        float ggx1 = GeometrySchlickGGX(n_dot_l, roughness);
        return ggx1 * ggx2;
    }
}


