struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos: vec3<f32>,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @builtin(vertex_index) vtx: u32,
}

struct ViewData {
    view: mat4x4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.pos = input.position;
    out.position = view_data.view * vec4f(input.position, 1.0);
    return out;
}

struct FragmentOutput {
    @location(0) color: vec4<f32>,
}

@group(0) @binding(1) var hdr_texture: texture_2d<f32>;
@group(0) @binding(2) var hdr_sampler: sampler;

fn reinhard(x: vec3<f32>) -> vec3<f32> {
    return x / (1.0 + x);
}

fn sample_spherical(v: vec3<f32>) -> vec2<f32> {
    let inv_atan = vec2f(0.1591, 0.3183);
    var uv = vec2f(atan2(v.z, v.x) , asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;
    let uv = sample_spherical(normalize(in.pos));

    // Ensure the image components are between 0.0 - 1.0
    let sdr = reinhard(textureSample(hdr_texture, hdr_sampler, uv).rgb);
    out.color = vec4f(sdr, 1.0);
    return out;
}
