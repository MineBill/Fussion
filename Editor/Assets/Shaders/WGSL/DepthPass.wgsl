struct VertexOutput {
    @builtin(position) position: vec4<f32>,
}

struct VertexInput {
    @builtin(instance_index) instance_index: u32,
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) tangent: vec4<f32>,
    @location(3) uv: vec2<f32>,
    @location(4) color: vec3<f32>,
}

struct WTF {
    model: mat4x4<f32>,
    light_space: mat4x4<f32>,
}

struct InstanceData {
    data: array<WTF>,
}

@group(0) @binding(0) var<storage, read> instance_data: InstanceData;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let model = instance_data.data[in.instance_index].model;
    let light_space = instance_data.data[in.instance_index].light_space;
    out.position = light_space * model * vec4f(in.position, 1.0);
    return out;
}