struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
    rotation: mat4x4f,
    position: vec4f,
}

struct DirectionalLight {
    light_space_matrix: array<mat4x4f, 4>,
    direction: vec4f,
    color: vec4f,
}

struct LightData {
    directional: DirectionalLight,

    shadow_split_distances: vec4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;
@group(0) @binding(1) var<uniform> light_data: LightData;

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos: vec3<f32>,
    @location(1) fsun: vec3<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) vtx: u32) -> VertexOutput {
    var out: VertexOutput;
    var positions: array<vec2<f32>, 4> = array(
        vec2f(-1.0,  1.0), vec2f(-1.0, -1.0),
        vec2f( 1.0,  1.0), vec2f( 1.0, -1.0)
    );

    out.position = vec4f(positions[vtx], 0.0, 1.0);
    let www = transpose(mat3x3f(view_data.view[0].xyz, view_data.view[1].xyz, view_data.view[2].xyz));
    out.pos = www * (inverse(view_data.projection) * out.position).xyz;
    out.fsun = vec3f(0.0, 0.05, 0.0);
    return out;
}

fn mod289(x: f32) -> f32 { return f32(x - floor(x * (1.0 / 289.0)) * 289.0); }
fn mod289_v(x: vec4f) -> vec4f { return x - floor(x * (1.0 / 289.0)) * 289.0; }
fn perm(x: vec4f) -> vec4f {return mod289_v(((x * 34.0) + 1.0) * x); }

fn noise(p: vec3f) -> f32 {
    let a = floor(p);
    var d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    let b = a.xxyy + vec4f(0.0, 1.0, 0.0, 1.0);
    let k1 = perm(b.xyxy);
    let k2 = perm(k1.xyxy + b.xyxy);

    let c = k2 + a.zzzz;
    let k3 = perm(c);
    let k4 = perm(c + 1.0);

    let o1 = fract(k3 * (1.0 / 41.0));
    let o2 = fract(k4 * (1.0 / 41.0));

    let o3 = o2 * d.z + o1 * (1.0 - d.z);
    let o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

const m = mat3x3f(0.0, 1.6, 1.2, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);

fn fbm(pp: vec3f) -> f32 {
    var p = pp;
    var f = 0.0;
    f += noise(p) / 2.0;
    p = m * p * 1.1;
    f += noise(p) / 4.0;
    p = m * p * 1.2;
    f += noise(p) / 6.0;
    p = m * p * 1.3;
    f += noise(p) / 12.0;
    p = m * p * 1.4;
    f += noise(p) / 24.0;
    return f;
}

struct GlobalData {
    time: f32,
}

struct FragmentOutput {
    @builtin(frag_depth) depth: f32,
    @location(0) color: vec4f,
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    if (in.pos.y < 0) {
        discard;
    }
    var out: FragmentOutput;
    var global_data: GlobalData;
    global_data.time = 1.0;

    let Br = 0.0025;
    let Bm = 0.0003;
    let g = 0.98;

    let nitrogen = vec3f(0.650, 0.570, 0.475);
    let Kr = Br / pow(nitrogen, vec3f(4.0));
    let Km = Bm / pow(nitrogen, vec3f(0.84));

    let mu = dot(normalize(in.pos), normalize(light_data.directional.direction.xyz));
    let rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    let mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    let day_extinction = exp(-exp(-((in.pos.y + light_data.directional.direction.xyz.y * 4.0) * (exp(-in.pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-in.pos.y * 16.0) + 0.1) * Kr / Br) * exp(-in.pos.y * exp(-in.pos.y * 8.0 ) * 4.0) * exp(-in.pos.y * 2.0) * 4.0;
    let night_extinction = vec3f(1.0 - exp(light_data.directional.direction.y)) * 0.2;
    let extinction = mix(day_extinction , night_extinction, -light_data.directional.direction.y * 0.2 + 0.5);
    var color = vec4f(rayleigh * mie * extinction, 1.0).xyz;

    let cirrus = 0.5;
    // Cirrus Clouds
    var density = smoothstep(1.0 - cirrus, 1.0, fbm(in.pos.xyz / in.pos.y * 1.0 + global_data.time * 0.01)) * 0.3;
    color = vec4f(mix(color.rgb, extinction * 10.0, density * max(in.pos.y, 0.0)), 1.0).xyz;

    density = smoothstep(1.0 - cirrus, 1.0, fbm(in.pos.xyz / in.pos.y * 4.0 + global_data.time * 0.01)) * 0.3;
    color = vec4f(mix(color.rgb, extinction * 10.0, density * max(in.pos.y, 0.0)), 1.0).xyz;

    // color = pow(1.0 - exp(-1.3 * color.rgb), vec3f(1.3));
    out.color = vec4f(color, 1.0);

    out.depth = 1.0;
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