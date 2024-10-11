#pragma once
import common;

[[vk::binding(0, 0)]]
ConstantBuffer<ViewData> uViewData;

// [[vk::binding(1, 0)]]
// ConstantBuffer<DebugOptions> uDebugOptions;