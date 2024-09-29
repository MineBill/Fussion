struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) thickness: f32,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) thickness: f32,
    @location(2) color: vec4<f32>,
}

struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    rotation: mat4x4f,
    position: vec4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.color = in.color;
    out.thickness = in.thickness;
    out.position = view_data.projection * view_data.view * vec4f(in.position, 1.0);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return in.color;
}