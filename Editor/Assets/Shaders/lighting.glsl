#ifndef LIGHTING_H
#define LIGHTING_H

#include "Defines.glsl"

#define MAX_SPOTLIGHTS  10
#define MAX_POOUTTLIGHTS 10

struct DirectionalLight {
    vec4 direction;
    vec4 color;

    mat4 light_space_matrix[4];
};

//struct SpotLight {
//    vec4 _padding;
//};

//struct PointLight {
//    vec4 color;
//    vec4 position;
//
//    float constant;
//    float linear;
//    float quadratic;
//    float _padding;
//};

layout(std140, set = SCENE_SET, binding = 1) uniform LightData {
    DirectionalLight directional;

//    PointLight pointlights[MAX_POOUTTLIGHTS];
//    SpotLight spotlights[MAX_SPOTLIGHTS];

    vec4 shadow_split_distances;
} u_LightData;
#endif // LIGHTING_H
