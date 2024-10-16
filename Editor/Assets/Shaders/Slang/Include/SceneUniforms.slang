#pragma once
import Common;

__generic<let sampleCount : int = 0, let format : int = 0>
    typealias Depth2DArray = __TextureImpl<
        float,
        __Shape2D,
        1, // isArray
        0, // isMS
        sampleCount,
        0, // access
        1, // isShadow
        0, // isCombined
        format>;

[[vk::binding(0, 1)]]
Texture2D uSSAOTex;
[[vk::binding(1, 1)]]
TextureCube uEnironmentTex;
[[vk::binding(2, 1)]]
SamplerState uSSAOSampler;
[[vk::binding(3, 1)]]
ConstantBuffer<LightData> uLightData;
[[vk::binding(4, 1)]]
Depth2DArray uShadowTex;


