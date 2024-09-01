#pragma once
#include "Enums.h"
#include <webgpu/wgpu.h>

namespace Fussion::GPU {
    auto ToWgpu(BackendRenderer backend) -> WGPUInstanceBackendFlags;

    auto ToWgpu(DevicePower power) -> WGPUPowerPreference;

    auto ToWgpu(PresentMode mode) -> WGPUPresentMode;

    auto ToWgpu(BufferUsageFlags usage) -> WGPUBufferUsageFlags;

    auto ToWgpu(TextureDimension dim) -> WGPUTextureDimension;

    auto ToWgpu(TextureViewDimension dim) -> WGPUTextureViewDimension;

    auto ToWgpu(TextureAspect aspect) -> WGPUTextureAspect;

    auto ToWgpu(LoadOp load) -> WGPULoadOp;
    auto ToWgpu(StoreOp store) -> WGPUStoreOp;

    auto ToWgpu(VertexStepMode mode) -> WGPUVertexStepMode;
    auto ToWgpu(PrimitiveTopology topology) -> WGPUPrimitiveTopology;
    auto ToWgpu(IndexFormat format) -> WGPUIndexFormat;

    auto ToWgpu(ColorWriteFlags flags) -> WGPUColorWriteMaskFlags;

    auto ToWgpu(ElementType type) -> WGPUVertexFormat;

    auto ToWgpu(FrontFace face) -> WGPUFrontFace;
    auto ToWgpu(Face face) -> WGPUCullMode;
    auto ToWgpu(PolygonMode mode) -> void;

    auto ToWgpu(CompareFunction compare) -> WGPUCompareFunction;
    auto ToWgpu(BlendOperation op) -> WGPUBlendOperation;
    auto ToWgpu(BlendFactor factor) -> WGPUBlendFactor;

    auto ToWgpu(StencilOperation op) -> WGPUStencilOperation;

    auto ToWgpu(SamplerBindingType type) -> WGPUSamplerBindingType;
    auto ToWgpu(StorageAccess access) -> WGPUStorageTextureAccess;

    auto ToWgpu(ShaderStageFlags flags) -> WGPUShaderStageFlags;

    auto FromWgpu(WGPUErrorType type) -> ErrorType;

    auto ToWgpu(TextureFormat format) -> WGPUTextureFormat;

    auto FromWgpu(WGPUTextureFormat format) -> TextureFormat;
}
