#pragma once
#include "Enums.h"
#include <webgpu/wgpu.h>

namespace Fussion::GPU {
    auto to_wgpu(BackendRenderer backend) -> WGPUInstanceBackendFlags;

    auto to_wgpu(Features feature) -> WGPUFeatureName;

    auto to_wgpu(QueryType set) -> WGPUQueryType;

    auto to_wgpu(DevicePower power) -> WGPUPowerPreference;

    auto to_wgpu(PresentMode mode) -> WGPUPresentMode;

    auto to_wgpu(BufferUsageFlags usage) -> WGPUBufferUsageFlags;

    auto to_wgpu(MapModeFlags mode) -> WGPUMapModeFlags;

    auto to_wgpu(TextureDimension dim) -> WGPUTextureDimension;

    auto to_wgpu(TextureViewDimension dim) -> WGPUTextureViewDimension;

    auto to_wgpu(TextureAspect aspect) -> WGPUTextureAspect;

    auto to_wgpu(LoadOp load) -> WGPULoadOp;
    auto to_wgpu(StoreOp store) -> WGPUStoreOp;

    auto to_wgpu(VertexStepMode mode) -> WGPUVertexStepMode;
    auto to_wgpu(PrimitiveTopology topology) -> WGPUPrimitiveTopology;
    auto to_wgpu(IndexFormat format) -> WGPUIndexFormat;

    auto to_wgpu(AddressMode mode) -> WGPUAddressMode;
    auto to_wgpu(FilterMode mode) -> WGPUFilterMode;

    auto to_wgpu(ColorWriteFlags flags) -> WGPUColorWriteMaskFlags;

    auto to_wgpu(ElementType type) -> WGPUVertexFormat;

    auto to_wgpu(FrontFace face) -> WGPUFrontFace;
    auto to_wgpu(Face face) -> WGPUCullMode;
    auto to_wgpu(PolygonMode mode) -> void;

    auto to_wgpu(CompareFunction compare) -> WGPUCompareFunction;
    auto to_wgpu(BlendOperation op) -> WGPUBlendOperation;
    auto to_wgpu(BlendFactor factor) -> WGPUBlendFactor;

    auto to_wgpu(TextureUsageFlags usage) -> WGPUTextureUsageFlags;

    auto to_wgpu(StencilOperation op) -> WGPUStencilOperation;

    auto to_wgpu(SamplerBindingType type) -> WGPUSamplerBindingType;
    auto to_wgpu(StorageAccess access) -> WGPUStorageTextureAccess;

    auto to_wgpu(ShaderStageFlags flags) -> WGPUShaderStageFlags;

    auto from_wgpu(WGPUErrorType type) -> ErrorType;

    auto to_wgpu(TextureFormat format) -> WGPUTextureFormat;

    auto from_wgpu(WGPUTextureFormat format) -> TextureFormat;

    auto from_wgpu(WGPUBufferMapState state) -> MapState;
}
