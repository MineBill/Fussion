struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    rotation: mat4x4f,
    position: vec4f,
    screen_size: vec2<f32>,
}

@group(0) @binding(0) var ssao_texture: texture_2d<f32>;
@group(0) @binding(1) var texture_sampler: sampler;

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
    let texel_size = 1.0 / vec2f(textureDimensions(ssao_texture, 0));

    var result = f32(0.0);
    for (var x = -2; x < 2; x++) {
        for (var y = -2; y < 2; y++) {
            var offset = vec2f(f32(x), f32(y)) * texel_size;
            offset.y *= -1.0;

            result += textureSample(ssao_texture, texture_sampler, in.uv + offset).r;
        }
    }

    // return textureSample(ssao_texture, texture_sampler, in.uv).r;
    return result / (16.0);
}