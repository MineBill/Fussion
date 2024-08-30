#version 450 core
#pragma samples: 8

#include "Global.glsl"
#include "Lighting.glsl"
#include "Scene.glsl"

#define PI 3.14159265359

layout (std140, set = OBJECT_SET, binding = 0) uniform Material {
    vec4 AlbedoColor;
    float Metallic;
    float Roughness;
} u_Material;

//layout(push_constant) uniform PushConstants {
//    mat4 model;
//} u_PushConstants;

struct VertexOutput {
    vec3 frag_color;
    vec2 frag_uv;
    vec4 frag_pos;
    vec3 normal;
    vec4 view_pos;

    vec3 tangent_light_dir;
    vec3 tangent_view_pos;
    vec3 tangent_frag_pos;
    mat3 TBN;

    vec4 pos_light_space[4];
};

#pragma type: vertex

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec2 a_UV;
layout (location = 4) in vec3 a_Color;

const mat4 shadow_bias = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0);

struct InstanceData {
    mat4 Model;
};

layout (std140, set = OBJECT_SET, binding = 6) readonly buffer InstanceBuffer {
    InstanceData Data[];
} b_InstanceBuffer;

layout (location = 0) out VertexOutput Out;
void Vertex() {
    Out.frag_color = a_Color;
    Out.frag_uv = a_UV;

    mat4 model = b_InstanceBuffer.Data[gl_InstanceIndex].Model;
    mat3 normal_matrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normal_matrix * a_Tangent.xyz);
    vec3 N = normalize(normal_matrix * a_Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 tbn = transpose(mat3(T, B, N));
    Out.TBN = tbn;

    gl_Position = u_ViewData.projection * u_ViewData.view * model * vec4(a_Position, 1.0);
    Out.frag_pos = vec4(model * vec4(a_Position, 1.0));
    Out.view_pos = u_ViewData.view * Out.frag_pos;

    Out.normal = N;
    Out.tangent_light_dir = tbn * u_LightData.directional.direction.xyz;
    Out.tangent_view_pos = tbn * u_SceneData.view_position.xyz;
    Out.tangent_frag_pos = tbn * Out.frag_pos.xyz;

    for (int i = 0; i < 4; i++) {
        Out.pos_light_space[i] = shadow_bias * u_LightData.directional.light_space_matrix[i] * Out.frag_pos;
    }
}

#pragma type: fragment

#include "PbrResources.glsl"

layout (location = 0) out vec4 o_Color;

layout (location = 0) in VertexOutput In;

float SampleShadow(float index, vec2 coords, float compare) {
    return step(compare, texture(uShadowMapArray, vec3(coords, index)).r);
}

float SampleShadowLinear(float index, vec2 coords, float compare, vec2 texel_size) {
    vec2 pp = coords / texel_size + vec2(0.5);
    vec2 fraction = fract(pp);
    vec2 texel = (pp - fraction) * texel_size;

    float a = SampleShadow(index, texel + vec2(0.0, 0.0), compare);
    float b = SampleShadow(index, texel + vec2(1.0, 0.0) * texel_size, compare);
    float c = SampleShadow(index, texel + vec2(0.0, 1.0) * texel_size, compare);
    float d = SampleShadow(index, texel + vec2(1.0, 1.0) * texel_size, compare);

    float aa = mix(a, c, fraction.y);
    float bb = mix(b, d, fraction.y);

    return mix(aa, bb, fraction.x);
}

float ShadowCalculation(int index) {
    float shadow = 0.0;
    vec4 fragPosLightSpace = In.pos_light_space[index];
    vec3 shadowCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w);
    if (shadowCoords.z > 1.0 || shadowCoords.z < -1.0) {
        return 1.0;
    }
    shadowCoords.y = 1.0 - shadowCoords.y;
    float bias = max((1.0 / 4096.0) * (1.0 - dot(In.normal, normalize(u_LightData.directional.direction.xyz))), 0.003);
    vec2 texel_size = vec2(1.0, 1.0) / textureSize(uShadowMapArray, 0).xy;

    const float SAMPLES = 3;
    const float SAMPLES_START = (SAMPLES - 1) / 2;
    const float SAMPLES_SQUARED = SAMPLES * SAMPLES;
    for (float x = -SAMPLES_START; x <= SAMPLES_START; x++) {
        for (float y = -SAMPLES_START; y <= SAMPLES_START; y++) {
            shadow += SampleShadowLinear(float(index), shadowCoords.xy + vec2(x, y) * texel_size, shadowCoords.z - bias, texel_size);
        }
    }

    return shadow / SAMPLES_SQUARED;
}

// f0:  Surface reflection at zero iradiance, when looking at the fragment directly.
//      Usually hardcoded as vec3(0.04) for non-metallic surfaces.
//      For metals, it's the 'tint'.
vec3 FrenselSchlick(float cos_theta, vec3 f0) {
    return f0 + (1 - f0) * pow(clamp(1 - cos_theta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 DoDirectionalLight() {
    vec3 N = texture(uNormalMap, In.frag_uv).rgb;
    N = normalize(N * 2.0 - 1.0);
    vec3 V = normalize(In.tangent_view_pos.xyz - In.tangent_frag_pos);
    vec3 L = normalize(In.tangent_light_dir);
    vec3 H = normalize(V + L);

    vec3 radiance = u_LightData.directional.color.rgb;
    vec3 F0 = vec3(0.04);
    vec3 albedo = texture(uAlbedoMap, In.frag_uv).rgb;

    float metalness = texture(uMetallicRoughnessMap, In.frag_uv).b * u_Material.Metallic;
    F0 = mix(F0, u_Material.AlbedoColor.rgb * albedo, metalness);

    vec3 F = FrenselSchlick(max(dot(H, V), 0.0), F0);

    float roughness = texture(uMetallicRoughnessMap, In.frag_uv).g * u_Material.Roughness;
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 ks = F;
    vec3 kd = vec3(1.0) - ks;
    kd *= 1.0 - metalness;

    vec3 I = In.frag_pos.xyz - u_SceneData.view_position.xyz;
    vec3 R = reflect(I, normalize(In.normal));
    vec3 reflection = vec3(1, 1, 1);

    float NdotL = max(dot(N, L), 0.0);

    //    float index = 0;
    //    if (1.0 - gl_FragCoord.z < u_LightData.shadow_split_distances.x) {
    //        index = 3;
    //    } else if (1.0 - gl_FragCoord.z < u_LightData.shadow_split_distances.y) {
    //        index = 2;
    //    } else if (1.0 - gl_FragCoord.z < u_LightData.shadow_split_distances.z) {
    //        index = 1;
    //    }

    #define SHADOW_CASCADES 4
    int index = 0;
    for (int i = 0; i < SHADOW_CASCADES - 1; i++) {
        if (In.view_pos.z < u_LightData.shadow_split_distances[i]) {
            index = i + 1;
        }
    }
    float shadow = ShadowCalculation(index);

    float occlusion = texture(uAmbientOcclusionMap, In.frag_uv).r;

    vec3 ambient = u_SceneData.ambient_color.rgb * albedo * u_Material.AlbedoColor.rgb * 0.2;
    ambient *= occlusion;
    vec3 ret = ambient + (kd * u_Material.AlbedoColor.rgb * albedo / PI + specular + reflection * metalness * ks) * radiance * NdotL * shadow * occlusion;

    ret += texture(uEmissiveMap, In.frag_uv).rgb;
    if (u_DebugOptions.shadow_cascade_colors == 1) {
        switch (int(index)) {
            case 0:
                ret *= vec3(1.0f, 0.25f, 0.25f);
                break;
            case 1:
                ret *= vec3(0.25f, 1.0f, 0.25f);
                break;
            case 2:
                ret *= vec3(0.25f, 0.25f, 1.0f);
                break;
            case 3:
                ret *= vec3(1.0f, 1.0f, 0.25f);
                break;
        }
    }
    return ret;
}

void Fragment() {
    vec3 Lo = vec3(0.0);

    Lo += DoDirectionalLight();

    o_Color = vec4(Lo, 1.0);
}

// vim:ft=glsl
