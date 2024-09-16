struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    position: vec4f,
    screen_size: vec2<f32>,
}

struct SSAOData {
    samples: array<vec3<f32>>,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

@group(1) @binding(0) var position_texture: texture_2d<f32>;
@group(1) @binding(1) var normal_texture: texture_2d<f32>;
@group(1) @binding(2) var noise_texture: texture_2d<f32>;
@group(1) @binding(3) var the_sampler: sampler;
@group(1) @binding(4) var<storage, read> ssao_data: SSAOData;

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) vtx: u32) -> VertexOutput {
    var out: VertexOutput;
    out.uv = vec2f(f32((vtx << 1) & 2), f32(vtx & 2));
    out.position = vec4f(out.uv * vec2f(2.0f, -2.0f) + vec2f( -1.0f, 1.0f), 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) f32 {
    let frag_pos = textureSample(position_texture, the_sampler, in.uv).xyz;
    let frag_normal = textureSample(normal_texture, the_sampler, in.uv).rgb;

    let noise_scale = view_data.screen_size / 4.0;
    let random_vec = textureSample(noise_texture, the_sampler, in.uv * noise_scale).xyz;

    let tangent = normalize(random_vec - frag_normal * dot(random_vec, frag_normal));
    let bitangent = cross(frag_normal, tangent);
    let tbn = mat3x3f(tangent, bitangent, frag_normal);

    let kernel_size = 64;
    let radius = 0.1;
    var occlusion: f32 = 0.0;
    for(var i = 0; i < kernel_size; i++) {
        var sample_pos = tbn * ssao_data.samples[i];
        sample_pos = frag_pos + sample_pos * radius;

        var offset = vec4f(sample_pos, 1.0);
        offset = view_data.projection * offset;
        offset = vec4f(offset.xyz / offset.w, 1.0);
        offset = vec4f(offset.xyz * 0.5 + 0.5, 1.0);
        offset.y *= -1.0;

        let sample_depth = textureSample(position_texture, the_sampler, offset.xy).z;
        let bias = 0.025;
        if (sample_depth >= sample_pos.z + bias) {
            let range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
            occlusion += range_check;
        }
    }
    return 1.0 - (occlusion / f32(kernel_size));
}