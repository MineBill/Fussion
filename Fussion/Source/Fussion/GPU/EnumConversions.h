#pragma once
#include "Enums.h"
#include <webgpu/wgpu.h>

namespace Fussion::GPU {
    auto ToWGPU(BackendRenderer backend) -> WGPUInstanceBackendFlags;

    auto ToWGPU(Feature feature) -> WGPUFeatureName;

    // auto to_wgpu(QueryType set) -> WGPUQueryType;

    auto ToWGPU(DevicePower power) -> WGPUPowerPreference;

    auto ToWGPU(PresentMode mode) -> WGPUPresentMode;

    auto ToWGPU(BufferUsageFlags usage) -> WGPUBufferUsageFlags;

    auto ToWGPU(MapModeFlags mode) -> WGPUMapModeFlags;

    auto ToWGPU(TextureDimension dim) -> WGPUTextureDimension;

    auto ToWGPU(TextureViewDimension dim) -> WGPUTextureViewDimension;

    auto ToWGPU(TextureAspect aspect) -> WGPUTextureAspect;

    auto ToWGPU(LoadOp load) -> WGPULoadOp;
    auto ToWGPU(StoreOp store) -> WGPUStoreOp;

    auto ToWGPU(VertexStepMode mode) -> WGPUVertexStepMode;
    auto ToWGPU(PrimitiveTopology topology) -> WGPUPrimitiveTopology;
    auto ToWGPU(IndexFormat format) -> WGPUIndexFormat;

    auto ToWGPU(AddressMode mode) -> WGPUAddressMode;
    auto ToWGPU(FilterMode mode) -> WGPUFilterMode;

    auto ToWGPU(ColorWriteFlags flags) -> WGPUColorWriteMaskFlags;

    auto ToWGPU(ElementType type) -> WGPUVertexFormat;

    auto ToWGPU(FrontFace face) -> WGPUFrontFace;
    auto ToWGPU(Face face) -> WGPUCullMode;
    auto ToWGPU(PolygonMode mode) -> void;

    auto ToWGPU(CompareFunction compare) -> WGPUCompareFunction;
    auto ToWGPU(BlendOperation op) -> WGPUBlendOperation;
    auto ToWGPU(BlendFactor factor) -> WGPUBlendFactor;

    auto ToWGPU(TextureUsageFlags usage) -> WGPUTextureUsageFlags;

    auto ToWGPU(StencilOperation op) -> WGPUStencilOperation;

    auto ToWGPU(SamplerBindingType type) -> WGPUSamplerBindingType;
    auto ToWGPU(StorageAccess access) -> WGPUStorageTextureAccess;

    auto ToWGPU(ShaderStageFlags flags) -> WGPUShaderStageFlags;

    auto from_wgpu(WGPUErrorType type) -> ErrorType;

    auto ToWGPU(TextureFormat format) -> WGPUTextureFormat;

    auto from_wgpu(WGPUTextureFormat format) -> TextureFormat;

    auto from_wgpu(WGPUBufferMapState state) -> MapState;
}
