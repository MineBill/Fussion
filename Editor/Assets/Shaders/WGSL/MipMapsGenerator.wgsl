struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) vtx: u32) -> VertexOutput {
    var out: VertexOutput;
    out.uv = vec2f(f32((vtx << u32(1)) & 2), f32(vtx & 2));
    out.position = vec4f(out.uv * vec2f(2.0f, -2.0f) + vec2f( -1.0f, 1.0f), 0.0f, 1.0f);
    return out;
}

@group(0) @binding(0) var tex: texture_2d<f32>;
@group(0) @binding(1) var sam: sampler;

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    let ss = textureSample(tex, sam, in.uv);
    let color = textureLoad;
    return ss + color;
}

rp