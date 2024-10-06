const pi = 3.14159265359;

const shadow_bias: mat4x4f = mat4x4f(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
);

struct VertexOutput {
    @builtin(position) position: vec4f,

    @location(0) frag_color: vec3f,
    @location(1) frag_uv: vec2f,
    @location(2) frag_pos: vec4f,

    @location(3) normal: vec3f,
    @location(4) view_pos: vec4f,

    @location(5) tangent: vec3f,
    @location(6) bitangent: vec3f,
    @location(7) normal: vec3f,

    @location(8) pos_light_space1: vec4f,
    @location(9) pos_light_space2: vec4f,
    @location(10) pos_light_space3: vec4f,
    @location(11) pos_light_space4: vec4f,
    // @location(5) tangent_light_dir: vec3f,
    // @location(6) tangent_view_pos: vec3f,
    // @location(7) tangent_frag_pos: vec3f,

    // @location(8) tbn: mat3x3f,

    // @location(9) pos_light_space: array<vec4f, 4>,
}

struct VertexInput {
    @builtin(instance_index) instance_index: u32,
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) tangent: vec4f,
    @location(3) uv: vec2f,
    @location(4) color: vec3f,
}

struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    rotation: mat4x4f,
    position: vec4f,
    screen_size: vec2f,
}

struct DirectionalLight {
    light_space_matrix: array<mat4x4f, 4>,
    direction: vec4f,
    color: vec4f,
    brightness: f32,
}

struct LightData {
    directional: DirectionalLight,

    shadow_split_distances: vec4f,
}

struct InstanceData {
    model: array<mat4x4f>,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;
@group(0) @binding(1) var<uniform> light_data: LightData;
@group(0) @binding(2) var shadow_texture: texture_depth_2d_array;

@group(1) @binding(0) var ssao_texture: texture_2d<f32>;
@group(1) @binding(1) var environment_map: texture_cube<f32>;
@group(1) @binding(2) var ssao_sampler: sampler;

@group(2) @binding(0) var<storage, read> instance_data: InstanceData;

// @group(1) @binding(1) var shadow_map_texture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let model = instance_data.model[in.instance_index];

    let model_inverse = inverse(model);
    let normal_matrix: mat3x3f = transpose(mat3x3f(model_inverse[0].xyz, model_inverse[1].xyz, model_inverse[2].xyz));
    let t = normalize(normal_matrix * in.tangent.xyz);
    let n = normalize(normal_matrix * in.normal);
    out.normal = n;
    out.tangent = normalize(t - dot(t, n) * n);
    out.bitangent = cross(n, out.tangent);

    out.frag_pos = vec4f(model * vec4f(in.position, 1.0));
    out.view_pos = view_data.view * out.frag_pos;

    out.frag_color = in.color;
    out.frag_uv = in.uv;
    out.position = view_data.projection * view_data.view * model * vec4f(in.position, 1.0);

    out.pos_light_space1 = shadow_bias * light_data.directional.light_space_matrix[0] * out.frag_pos;
    out.pos_light_space2 = shadow_bias * light_data.directional.light_space_matrix[1] * out.frag_pos;
    out.pos_light_space3 = shadow_bias * light_data.directional.light_space_matrix[2] * out.frag_pos;
    out.pos_light_space4 = shadow_bias * light_data.directional.light_space_matrix[3] * out.frag_pos;

    return out;
}

//fn sample_shadow(index: u32, coords: vec2f, compare: f32) -> f32 {
//    return step(compare, textureSample(shadow_texture, shadow_sampler, coords, index));
//}
//
//fn sample_shadow_linear(index: u32, coords: vec2<f32>, compare: f32, texel_size: vec2<f32>) -> f32 {
//    let pp = coords / texel_size + vec2f(0.5);
//    let fraction = fract(pp);
//    let texel = (pp - fraction) * texel_size;
//
//    let a = sample_shadow(index, texel, compare);
//    let b = sample_shadow(index, texel + vec2f(1.0, 0.0) * texel_size, compare);
//    let c = sample_shadow(index, texel + vec2f(0.0, 1.0) * texel_size, compare);
//    let d = sample_shadow(index, texel + vec2f(1.0, 1.0) * texel_size, compare);
//
//    let aa = mix(a, c, fraction.y);
//    let bb = mix(b, d, fraction.y);
//
//    return mix(aa, bb, fraction.x);
//}

fn sample_shadow(index: u32, coords: vec2<f32>, compare: f32, texel_size: vec2<f32>) -> f32 {
    var result = 0.0f;
    let samples = 16;
    for (var i = 0; i < samples; i++) {
        let z = textureSampleCompare(shadow_texture, shadow_sampler, coords + sample_poisson(i) * texel_size * 2.0f, index, compare);
        result += z;
    }
    return result / f32(samples);
}

fn do_directional_shadow(in: VertexOutput, light: DirectionalLight, index: u32) -> f32 {
    var shadow = 0.0;
    // wgsl in all it's glory :)
    var pos_light_space = mat4x4f(
        in.pos_light_space1,
        in.pos_light_space2,
        in.pos_light_space3,
        in.pos_light_space4
    );
    var i = u32(0);
    var frag_pos_light_space: vec4f = pos_light_space[0];
    if (index == 1) {
        frag_pos_light_space = pos_light_space[1];
    } else if (index == 2) {
        frag_pos_light_space = pos_light_space[2];
    } else if (index == 3) {
        frag_pos_light_space = pos_light_space[3];
    }
    var coords = (frag_pos_light_space.xyz / frag_pos_light_space.w);
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
    let bias = max((1.0 / 4096.0) * (1.0 - dot(in.normal, normalize(light.direction.xyz))), 0.003);
    let texel_size = vec2f(1.0, 1.0) / vec2f(textureDimensions(shadow_texture));

    return sample_shadow(index, coords.xy, coords.z - bias, texel_size);
}

fn frensel_schlick(cos_theta: f32, f0: vec3f, roughness: f32) -> vec3f {
    return f0 + (max(vec3f(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

fn distribution_ggx(n: vec3f, h: vec3f, roughness: f32) -> f32 {
    let a = roughness * roughness;
    let a2 = a * a;
    let n_dot_h = max(dot(n, h), 0.0);
    let n_dot_h2 = n_dot_h * n_dot_h;

    let num = a2;
    var denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;

    return num / denom;
}

fn geometry_schlick_ggx(n_dot_v: f32, roughness: f32) -> f32 {
    let r = roughness + 1.0;
    let k = (r * r) / 8.0;

    let num = n_dot_v;
    let denom = n_dot_v * (1.0 - k) + k;

    return num / denom;
}

fn geometry_smith(n: vec3f, v: vec3f, l: vec3f, roughness: f32) -> f32 {
    let n_dot_v = max(dot(n, v), 0.0);
    let n_dot_l = max(dot(n, l), 0.0);
    let ggx2 = geometry_schlick_ggx(n_dot_v, roughness);
    let ggx1 = geometry_schlick_ggx(n_dot_l, roughness);
    return ggx1 * ggx2;
}

struct Material {
    albedo_color: vec4f,
    metallic: f32,
    roughness: f32,
    tiling: vec2<f32>,
}

@group(2) @binding(1) var<uniform> material: Material;
@group(2) @binding(2) var albedo_map: texture_2d<f32>;
@group(2) @binding(3) var normal_map: texture_2d<f32>;
@group(2) @binding(4) var metallic_roughness_map: texture_2d<f32>;
@group(2) @binding(5) var occlusion_map: texture_2d<f32>;
@group(2) @binding(6) var emissive_map: texture_2d<f32>;
@group(2) @binding(7) var linear_sampler: sampler;
@group(2) @binding(8) var shadow_sampler: sampler_comparison;

fn do_directional_light(light: DirectionalLight, in: VertexOutput) -> vec3f {
    var n = textureSample(normal_map, linear_sampler, in.frag_uv * material.tiling).rgb;
    n = normalize(n * 2.0 - 1.0);

    let tbn = transpose(mat3x3f(in.tangent, in.bitangent, in.normal));
    let tangent_view_pos = tbn * view_data.position.xyz;
    let tangent_frag_pos = tbn * in.frag_pos.xyz;
    let tangent_light_dir = tbn * light.direction.xyz;

    let v = normalize(tangent_view_pos - tangent_frag_pos);
    let l = normalize(tangent_light_dir);
    let h = normalize(v + l);

    let radiance = light.color.rgb * light.brightness;
    let albedo = textureSample(albedo_map, linear_sampler, in.frag_uv * material.tiling).rgb * material.albedo_color.rgb;
    let metalness = textureSample(metallic_roughness_map, linear_sampler, in.frag_uv * material.tiling).b * material.metallic;
    var f0 = mix(vec3f(0.04), albedo, metalness);

    let roughness = textureSample(metallic_roughness_map, linear_sampler, in.frag_uv * material.tiling).g * material.roughness;
    let ndf = distribution_ggx(n, h, roughness);
    let g = geometry_smith(n, v, l, roughness);

    let f = frensel_schlick(max(dot(h, v), 0.0), f0, roughness);
    let numerator = ndf * g * f;
    let denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
    let specular = numerator / denominator;

    let ks = f;
    var kd = vec3f(1.0) - ks;
    kd *= 1.0 - metalness;

    let i = in.frag_pos.xyz - view_data.position.xyz;
    let r = reflect(i, normalize(in.normal));
    let reflection = vec3f(1.0, 1.0, 1.0);

    let ndotl = max(dot(n, l), 0.0);

    var index: u32 = 0;
    for (var i = u32(0); i < 4; i++) {
        if (in.view_pos.z < light_data.shadow_split_distances[i]) {
            index = i + 1;
        }
    }

    let shadow = clamp(do_directional_shadow(in, light, index), 0.0, 1.0);

    let occlusion = textureSample(occlusion_map, linear_sampler, in.frag_uv * material.tiling).r;
    let ssao_occlusion = textureSample(ssao_texture, linear_sampler, in.position.xy / view_data.screen_size).r;

    let irradiance = textureSample(environment_map, linear_sampler, n).rgb;
    let diffuse = irradiance * albedo;
    var ambient = (kd * diffuse) * occlusion * ssao_occlusion;

    var ret = ambient + (kd * albedo / pi + specular) * radiance * ndotl * shadow;

    ret += textureSample(emissive_map, linear_sampler, in.frag_uv * material.tiling).rgb;

    return ret;
}

struct FragmentOutput {
    @location(0) color: vec4f,
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;

    var lo = vec3f(0.0);
    lo += do_directional_light(light_data.directional, in);

//    let gamma = 2.2;
//    let exposure = 1.0;
//    var mapped = vec3f(1.0) - exp(-lo.rgb * exposure);
//    mapped = pow(mapped, vec3f(1.0 / gamma));
//    lo = mapped;

    out.color = vec4f(lo, 1.0);
    return out;
}

fn inverse(m: mat4x4f) -> mat4x4f {
    let a00 = m[0][0]; let a01 = m[0][1]; let a02 = m[0][2]; let a03 = m[0][3];
    let a10 = m[1][0]; let a11 = m[1][1]; let a12 = m[1][2]; let a13 = m[1][3];
    let a20 = m[2][0]; let a21 = m[2][1]; let a22 = m[2][2]; let a23 = m[2][3];
    let a30 = m[3][0]; let a31 = m[3][1]; let a32 = m[3][2]; let a33 = m[3][3];

    let b00 = a00 * a11 - a01 * a10;
    let b01 = a00 * a12 - a02 * a10;
    let b02 = a00 * a13 - a03 * a10;
    let b03 = a01 * a12 - a02 * a11;
    let b04 = a01 * a13 - a03 * a11;
    let b05 = a02 * a13 - a03 * a12;
    let b06 = a20 * a31 - a21 * a30;
    let b07 = a20 * a32 - a22 * a30;
    let b08 = a20 * a33 - a23 * a30;
    let b09 = a21 * a32 - a22 * a31;
    let b10 = a21 * a33 - a23 * a31;
    let b11 = a22 * a33 - a23 * a32;

    let det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    return mat4x4f(
        a11 * b11 - a12 * b10 + a13 * b09,
        a02 * b10 - a01 * b11 - a03 * b09,
        a31 * b05 - a32 * b04 + a33 * b03,
        a22 * b04 - a21 * b05 - a23 * b03,
        a12 * b08 - a10 * b11 - a13 * b07,
        a00 * b11 - a02 * b08 + a03 * b07,
        a32 * b02 - a30 * b05 - a33 * b01,
        a20 * b05 - a22 * b02 + a23 * b01,
        a10 * b10 - a11 * b08 + a13 * b06,
        a01 * b08 - a00 * b10 - a03 * b06,
        a30 * b04 - a31 * b02 + a33 * b00,
        a21 * b02 - a20 * b04 - a23 * b00,
        a11 * b07 - a10 * b09 - a12 * b06,
        a00 * b09 - a01 * b07 + a02 * b06,
        a31 * b01 - a30 * b03 - a32 * b00,
        a20 * b03 - a21 * b01 + a22 * b00) * (1 / det);
}


fn sample_poisson(index: i32) -> vec2<f32> {
    var poisson: array<vec2<f32>, 64> = array(
        vec2f(-0.884081, 0.124488),
        vec2f(-0.714377, 0.027940),
        vec2f(-0.747945, 0.227922),
        vec2f(-0.939609, 0.243634),
        vec2f(-0.985465, 0.045534),
        vec2f(-0.861367, -0.136222),
        vec2f(-0.881934, 0.396908),
        vec2f(-0.466938, 0.014526),
        vec2f(-0.558207, 0.212662),
        vec2f(-0.578447, -0.095822),
        vec2f(-0.740266, -0.095631),
        vec2f(-0.751681, 0.472604),
        vec2f(-0.553147, -0.243177),
        vec2f(-0.674762, -0.330730),
        vec2f(-0.402765, -0.122087),
        vec2f(-0.319776, -0.312166),
        vec2f(-0.413923, -0.439757),
        vec2f(-0.979153, -0.201245),
        vec2f(-0.865579, -0.288695),
        vec2f(-0.243704, -0.186378),
        vec2f(-0.294920, -0.055748),
        vec2f(-0.604452, -0.544251),
        vec2f(-0.418056, -0.587679),
        vec2f(-0.549156, -0.415877),
        vec2f(-0.238080, -0.611761),
        vec2f(-0.267004, -0.459702),
        vec2f(-0.100006, -0.229116),
        vec2f(-0.101928, -0.380382),
        vec2f(-0.681467, -0.700773),
        vec2f(-0.763488, -0.543386),
        vec2f(-0.549030, -0.750749),
        vec2f(-0.809045, -0.408738),
        vec2f(-0.388134, -0.773448),
        vec2f(-0.429392, -0.894892),
        vec2f(-0.131597, 0.065058),
        vec2f(-0.275002, 0.102922),
        vec2f(-0.106117, -0.068327),
        vec2f(-0.294586, -0.891515),
        vec2f(-0.629418, 0.379387),
        vec2f(-0.407257, 0.339748),
        vec2f(0.071650, -0.384284),
        vec2f(0.022018, -0.263793),
        vec2f(0.003879, -0.136073),
        vec2f(-0.137533, -0.767844),
        vec2f(-0.050874, -0.906068),
        vec2f(0.114133, -0.070053),
        vec2f(0.163314, -0.217231),
        vec2f(-0.100262, -0.587992),
        vec2f(-0.004942, 0.125368),
        vec2f(0.035302, -0.619310),
        vec2f(0.195646, -0.459022),
        vec2f(0.303969, -0.346362),
        vec2f(-0.678118, 0.685099),
        vec2f(-0.628418, 0.507978),
        vec2f(-0.508473, 0.458753),
        vec2f(0.032134, -0.782030),
        vec2f(0.122595, 0.280353),
        vec2f(-0.043643, 0.312119),
        vec2f(0.132993, 0.085170),
        vec2f(-0.192106, 0.285848),
        vec2f(0.183621, -0.713242),
        vec2f(0.265220, -0.596716),
        vec2f(-0.009628, -0.483058),
        vec2f(-0.018516, 0.435703)
    );
    return poisson[index % 64];
}
