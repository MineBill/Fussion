struct VertexOutput {
    @builtin(position) position: vec4f,

    @location(0) near_point: vec3f,
    @location(1) far_point: vec3f,
    @location(2) proj1: vec4f,
    @location(3) proj2: vec4f,
    @location(4) proj3: vec4f,
    @location(5) proj4: vec4f,

    @location(6) view1: vec4f,
    @location(7) view2: vec4f,
    @location(8) view3: vec4f,
    @location(9) view4: vec4f,
    // @location(3) view: mat4x4<f32>,
};

struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

fn unproject_point(x: f32, y: f32, z: f32, view: mat4x4f, projection: mat4x4f) -> vec3f {
    let v = inverse(view);
    let p = inverse(projection);
    let point = v * p * vec4f(x, y, z, 1.0);
    return point.xyz / point.w;
}

@vertex
fn vs_main(@builtin(vertex_index) vtx: u32) -> VertexOutput {
    var out = VertexOutput();

    var plane: array<vec3f, 6> = array(
        vec3f(-1, -1, 0), vec3f(1, -1, 0), vec3f(-1, 1, 0),
        vec3f(-1, 1, 0), vec3f(1, -1, 0), vec3f(1, 1, 0)
    );
    let p = plane[vtx].xyz;
    out.position = vec4f(p, 1.0);

    out.proj1 = view_data.projection[0];
    out.proj2 = view_data.projection[1];
    out.proj3 = view_data.projection[2];
    out.proj4 = view_data.projection[3];

    out.view1 = view_data.view[0];
    out.view2 = view_data.view[1];
    out.view3 = view_data.view[2];
    out.view4 = view_data.view[3];

    out.near_point = unproject_point(p.x, p.y, -1.0, view_data.view, view_data.projection);
    out.far_point = unproject_point(p.x, p.y, 1.0, view_data.view, view_data.projection);
    return out;
}

fn grid(frag_pos: vec3f, scale: f32) -> vec4f {
    let coord = frag_pos.xz * scale;
    let derivative = fwidth(coord);
    let grid = abs(fract(coord - 0.5) - 0.5) / derivative;

    let line = min(grid.x, grid.y);
    let minz = min(derivative.y, 1.0);
    let minx = min(derivative.x, 1.0);

    var color = vec4f(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    if (frag_pos.x > -0.5 * minx && frag_pos.x < 0.5 * minx) {
        color.x = 3.0 / 255.0;
        color.y = 51.0 / 255.0;
        color.z = 202.0 / 255.0;
    }

    if (frag_pos.x > -0.5 * minx && frag_pos.x < 0.5 * minx) {
        color.x = 237.0 / 255.0;
        color.y = 35.0 / 255.0;
        color.z = 35.0 / 255.0;
    }

    return color;
}

fn compute_depth(pos: vec3f, view: mat4x4f, projection: mat4x4f) -> f32 {
    let clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    let t = (clip_space_pos.z / clip_space_pos.w);
    return t;
}

const near: f32 = 0.1;
const far: f32 = 1000.0;

fn compute_linear_depth(pos: vec3f, view: mat4x4f, projection: mat4x4f) -> f32 {
    let clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    let clip_space_depth = clip_space_pos.z / clip_space_pos.w;
    let linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linear_depth / far;
}

struct FragmentOutput {
    @builtin(frag_depth) depth: f32,

    @location(0) color0: vec4f,
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;

    let view = mat4x4f(
        in.view1,
        in.view2,
        in.view3,
        in.view4,
    );

    let projection = mat4x4f(
        in.proj1,
        in.proj2,
        in.proj3,
        in.proj4,
    );

    let t = (-in.near_point.y) / (in.far_point.y - in.near_point.y);
    let frag_pos = in.near_point + t * (in.far_point - in.near_point);

    let d = compute_depth(frag_pos, view, projection);
    out.depth = d;

    let linear_depth = compute_linear_depth(frag_pos, view, projection);
    let fading = max(0.0, 0.5 - linear_depth);

    out.color0 = (grid(frag_pos, 1.0) + grid(frag_pos, 0.1)) * f32(t > 0);
    out.color0.a *= fading;
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