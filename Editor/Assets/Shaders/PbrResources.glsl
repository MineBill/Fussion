#ifndef PBR_RESOURCES_H
#define PBR_RESOURCES_H

#include "Defines.glsl"

layout(set = OBJECT_SET, binding = 1) uniform sampler2D uAlbedoMap;
layout(set = OBJECT_SET, binding = 2) uniform sampler2D uNormalMap;
layout(set = OBJECT_SET, binding = 3) uniform sampler2D uAmbientOcclusionMap;
layout(set = OBJECT_SET, binding = 4) uniform sampler2D uMetallicRoughnessMap;
layout(set = OBJECT_SET, binding = 5) uniform sampler2D uEmissiveMap;

layout(set = SCENE_SET,  binding = 2) uniform sampler2DArray uShadowMapArray;

#endif // PBR_RESOURCES_H