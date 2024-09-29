struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos: vec3<f32>,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @builtin(vertex_index) vtx: u32,
}

struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    rotation: mat4x4f,
    position: vec4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.pos = input.position;
    out.position = view_data.projection * view_data.rotation * vec4f(input.position, 1.0);
    return out;
}

struct FragmentOutput {
    @location(0) color: vec4<f32>,
}

@group(2) @binding(0) var hdr_texture: texture_cube<f32>;
@group(2) @binding(1) var hdr_sampler: sampler;

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
//    let uv = sample_spherical(normalize(in.pos));
    out.color = vec4f(textureSample(hdr_texture, hdr_sampler, normalize(in.pos)).rgb, 1.0);
    return out;
}
