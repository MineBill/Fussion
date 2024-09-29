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

@group(0) @binding(1) var cube_texture: texture_cube<f32>;
@group(0) @binding(2) var cube_sampler: sampler;

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

    let normal = normalize(in.pos);
    var irradiance = vec3f(0.0);

    var up = vec3f(0, 1, 0);
    let right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    let PI = 3.14159265359;
    let sample_delta = 0.025;
    var nr_samples = 0;
    for (var phi = 0.0; phi < 2.0 * PI; phi += sample_delta) {
        for (var theta = 0.0; theta < 0.5 * PI; theta += sample_delta) {
            let tangent_sample = vec3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            let sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += textureSample(cube_texture, cube_sampler, normalize(sample_vec)).rgb * cos(theta) * sin(theta);
            nr_samples += 1;
        }
    }
    irradiance = PI * irradiance * (1.0 / f32(nr_samples));
    out.color = vec4f(irradiance, 1.0);
    return out;
}
