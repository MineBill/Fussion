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

struct FragmentOutput {
    @location(0) color: vec4<f32>,
}

struct TonemappingSettings {
    gamma: f32,
    exposure: f32,
    mode: u32,
}

@group(0) @binding(0) var hdr_texture: texture_2d<f32>;
@group(0) @binding(1) var hdr_sampler: sampler;
@group(0) @binding(2) var<uniform> settings: TonemappingSettings;

// Maps HDR values to linear values
// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
fn aces(hdr: vec3<f32>) -> vec3<f32> {
    let m1 = mat3x3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777,
    );
    let m2 = mat3x3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602,
    );
    let v = m1 * hdr;
    let a = v * (v + 0.0245786) - 0.000090537;
    let b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return clamp(m2 * (a / b), vec3(0.0), vec3(1.0));
}

fn reinhard(x: vec3<f32>) -> vec3<f32> {
    return x / (1.0 + x);
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;
    let hdr = textureSample(hdr_texture, hdr_sampler, in.uv);
    var sdr = hdr.rgb * settings.exposure;

    if (settings.mode == 1) {
        sdr = aces(sdr.rgb);
    } else if (settings.mode == 2) {
        sdr = reinhard(sdr.rgb);
    }

    sdr = pow(sdr, vec3f(1.0f / settings.gamma));

    out.color = vec4f(sdr, 1.0);
    return out;
}
