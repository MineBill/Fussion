#pragma once
import Common;

[[vk::binding(0, 1)]]
Texture2D uSSAOTex;
[[vk::binding(1, 1)]]
TextureCube uEnvironmentTex;
[[vk::binding(2, 1)]]
SamplerState uSSAOSampler;
[[vk::binding(3, 1)]]
ConstantBuffer<LightData> uLightData;
[[vk::binding(4, 1)]]
Depth2DArray uShadowTex;


