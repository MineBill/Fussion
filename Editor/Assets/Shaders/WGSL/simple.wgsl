#import "Global.wgsl"

struct ViewData {
    projection: mat4x4f,
    view: mat4x4f,
}

@group(0) @binding(0) var<uniform> view_data: ViewData;

@vertex
fn vs_main(@builtin(vertex_index) vertex_index : u32) -> @builtin(position) vec4f {
  var pos = array(
    vec2( 0.0,  0.5),
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5)
  );

  return view_data.projection * view_data.view * vec4(pos[vertex_index], 0, 1);
}

@fragment
fn fs_main() -> @location(0) vec4f {
  return vec4(1, sin(1.0 / 128.0), 0, 1);
}
