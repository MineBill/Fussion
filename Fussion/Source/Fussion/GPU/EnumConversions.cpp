#include "FussionPCH.h"
#include "EnumConversions.h"
#include "Enums.h"

#include <webgpu/wgpu.h>

namespace Fussion::GPU {
    auto ToWgpu(BackendRenderer backend) -> WGPUInstanceBackendFlags
    {
        switch (backend) {
        case BackendRenderer::Vulkan:
            return WGPUInstanceBackend_Vulkan;
        case BackendRenderer::DX12:
            return WGPUInstanceBackend_DX12;
        case BackendRenderer::OpenGL:
            return WGPUInstanceBackend_GL;
        }
        UNREACHABLE;
    }

    auto ToWgpu(DevicePower power) -> WGPUPowerPreference
    {
        switch (power) {
        case DevicePower::Undefined:
            return WGPUPowerPreference_Undefined;
        case DevicePower::LowPower:
            return WGPUPowerPreference_LowPower;
        case DevicePower::HighPerformance:
            return WGPUPowerPreference_HighPerformance;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(PresentMode mode) -> WGPUPresentMode
    {
        switch (mode) {
        case PresentMode::Fifo:
            return WGPUPresentMode_Fifo;
        case PresentMode::FifoRelaxed:
            return WGPUPresentMode_FifoRelaxed;
        case PresentMode::Immediate:
            return WGPUPresentMode_Immediate;
        case PresentMode::Mailbox:
            return WGPUPresentMode_Mailbox;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(BufferUsageFlags usage) -> WGPUBufferUsageFlags
    {
        using enum BufferUsage;
        WGPUBufferUsageFlags result{};
        if (usage.Test(None))
            result |= WGPUBufferUsage_None;
        if (usage.Test(MapRead))
            result |= WGPUBufferUsage_MapRead;
        if (usage.Test(MapWrite))
            result |= WGPUBufferUsage_MapWrite;
        if (usage.Test(CopySrc))
            result |= WGPUBufferUsage_CopySrc;
        if (usage.Test(CopyDst))
            result |= WGPUBufferUsage_CopyDst;
        if (usage.Test(Index))
            result |= WGPUBufferUsage_Index;
        if (usage.Test(Vertex))
            result |= WGPUBufferUsage_Vertex;
        if (usage.Test(Uniform))
            result |= WGPUBufferUsage_Uniform;
        if (usage.Test(Storage))
            result |= WGPUBufferUsage_Storage;
        if (usage.Test(Indirect))
            result |= WGPUBufferUsage_Indirect;
        if (usage.Test(QueryResolve))
            result |= WGPUBufferUsage_QueryResolve;

        return result;
    }

    auto ToWgpu(TextureDimension dim) -> WGPUTextureDimension
    {
        switch (dim) {
        case TextureDimension::D1:
            return WGPUTextureDimension_1D;
        case TextureDimension::D2:
            return WGPUTextureDimension_2D;
        case TextureDimension::D3:
            return WGPUTextureDimension_3D;
        }
        UNREACHABLE;
    }

    auto ToWgpu(TextureViewDimension dim) -> WGPUTextureViewDimension
    {
        switch (dim) {
        case TextureViewDimension::Undefined:
            return WGPUTextureViewDimension_Undefined;
        case TextureViewDimension::D1:
            return WGPUTextureViewDimension_1D;
        case TextureViewDimension::D2:
            return WGPUTextureViewDimension_2D;
        case TextureViewDimension::D2_Array:
            return WGPUTextureViewDimension_2DArray;
        case TextureViewDimension::Cube:
            return WGPUTextureViewDimension_Cube;
        case TextureViewDimension::CubeArray:
            return WGPUTextureViewDimension_CubeArray;
        case TextureViewDimension::D3:
            return WGPUTextureViewDimension_3D;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(TextureAspect aspect) -> WGPUTextureAspect
    {
        switch (aspect) {
        case TextureAspect::All:
            return WGPUTextureAspect_All;
        case TextureAspect::StencilOnly:
            return WGPUTextureAspect_StencilOnly;
        case TextureAspect::DepthOnly:
            return WGPUTextureAspect_DepthOnly;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(LoadOp load) -> WGPULoadOp
    {
        switch (load) {
        case LoadOp::Undefined:
            return WGPULoadOp_Undefined;
        case LoadOp::Clear:
            return WGPULoadOp_Clear;
        case LoadOp::Load:
            return WGPULoadOp_Clear;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(SamplerBindingType type) -> WGPUSamplerBindingType
    {
        switch (type) {
        case SamplerBindingType::Filtering:
            return WGPUSamplerBindingType_Filtering;
        case SamplerBindingType::NonFiltering:
            return WGPUSamplerBindingType_NonFiltering;
        case SamplerBindingType::Comparison:
            return WGPUSamplerBindingType_Comparison;
        }
        UNREACHABLE;
    }

    auto ToWgpu(StorageAccess access) -> WGPUStorageTextureAccess
    {
        switch (access) {
        case StorageAccess::WriteOnly:
            return WGPUStorageTextureAccess_WriteOnly;
        case StorageAccess::ReadOnly:
            return WGPUStorageTextureAccess_ReadOnly;
        case StorageAccess::ReadWrite:
            return WGPUStorageTextureAccess_ReadWrite;
        }
        UNREACHABLE;
    }

    auto ToWgpu(ShaderStageFlags flags) -> WGPUShaderStageFlags
    {
        WGPUShaderStageFlags result = WGPUShaderStage_None;
        if (flags.Test(ShaderStage::Vertex))
            result |= WGPUShaderStage_Vertex;
        if (flags.Test(ShaderStage::Fragment))
            result |= WGPUShaderStage_Fragment;
        if (flags.Test(ShaderStage::Compute))
            result |= WGPUShaderStage_Compute;
        return result;
    }

    auto ToWgpu(StoreOp store) -> WGPUStoreOp
    {
        switch (store) {
        case StoreOp::Undefined:
            return WGPUStoreOp_Undefined;
        case StoreOp::Store:
            return WGPUStoreOp_Store;
        case StoreOp::Discard:
            return WGPUStoreOp_Discard;
        }
    }

    auto ToWgpu(VertexStepMode mode) -> WGPUVertexStepMode
    {
        switch (mode) {
        case VertexStepMode::Vertex:
            return WGPUVertexStepMode_Vertex;
        case VertexStepMode::Instance:
            return WGPUVertexStepMode_Instance;
        }
        UNREACHABLE;
    }

    auto ToWgpu(PrimitiveTopology topology) -> WGPUPrimitiveTopology
    {
        switch (topology) {
        case PrimitiveTopology::PointList:
            return WGPUPrimitiveTopology_PointList;
        case PrimitiveTopology::LineList:
            return WGPUPrimitiveTopology_LineList;
        case PrimitiveTopology::LineStrip:
            return WGPUPrimitiveTopology_LineStrip;
        case PrimitiveTopology::TriangleList:
            return WGPUPrimitiveTopology_TriangleList;
        case PrimitiveTopology::TriangleStrip:
            return WGPUPrimitiveTopology_TriangleStrip;
        }
        UNREACHABLE;
    }

    auto ToWgpu(IndexFormat format) -> WGPUIndexFormat
    {
        switch (format) {
        case IndexFormat::U16:
            return WGPUIndexFormat_Uint16;
        case IndexFormat::U32:
            return WGPUIndexFormat_Uint32;
        }
        UNREACHABLE;
    }

    auto ToWgpu(ColorWriteFlags flags) -> WGPUColorWriteMaskFlags
    {
        WGPUColorWriteMaskFlags result = 0;
        if (flags.Test(ColorWrite::Green))
            result |= WGPUColorWriteMask_Green;
        if (flags.Test(ColorWrite::Red))
            result |= WGPUColorWriteMask_Red;
        if (flags.Test(ColorWrite::Blue))
            result |= WGPUColorWriteMask_Blue;
        if (flags.Test(ColorWrite::Green))
            result |= WGPUColorWriteMask_Green;
        return result;
    }

    auto ToWgpu(ElementType type) -> WGPUVertexFormat
    {
        switch (type) {
        case ElementType::Int:
            return WGPUVertexFormat_Sint32;
        case ElementType::Int2:
            return WGPUVertexFormat_Sint32x2;
        case ElementType::Int3:
            return WGPUVertexFormat_Sint32x3;
        case ElementType::Int4:
            return WGPUVertexFormat_Sint32x4;
        case ElementType::Float:
            return WGPUVertexFormat_Float32;
        case ElementType::Float2:
            return WGPUVertexFormat_Float32x2;
        case ElementType::Float3:
            return WGPUVertexFormat_Float32x3;
        case ElementType::Float4:
            return WGPUVertexFormat_Float32x4;
        case ElementType::Mat3:
            return WGPUVertexFormat_Float32x3;
        case ElementType::Mat4:
            return WGPUVertexFormat_Float32x4;
        }
        UNREACHABLE;
    }

    auto ToWgpu(FrontFace face) -> WGPUFrontFace
    {
        switch (face) {
        case FrontFace::Ccw:
            return WGPUFrontFace_CCW;
        case FrontFace::Cw:
            return WGPUFrontFace_CW;
        }
        UNREACHABLE;
    }

    auto ToWgpu(Face face) -> WGPUCullMode
    {
        switch (face) {
        case Face::None:
            return WGPUCullMode_None;
        case Face::Front:
            return WGPUCullMode_Front;
        case Face::Back:
            return WGPUCullMode_Back;
        }
        UNREACHABLE;
    }

    auto ToWgpu(PolygonMode mode) -> void
    {
        UNREACHABLE;
    }

    auto ToWgpu(CompareFunction compare) -> WGPUCompareFunction
    {
        switch (compare) {
        case CompareFunction::Never:
            return WGPUCompareFunction_Never;
        case CompareFunction::Less:
            return WGPUCompareFunction_Less;
        case CompareFunction::Equal:
            return WGPUCompareFunction_Equal;
        case CompareFunction::LessEqual:
            return WGPUCompareFunction_LessEqual;
        case CompareFunction::Greater:
            return WGPUCompareFunction_Greater;
        case CompareFunction::NotEqual:
            return WGPUCompareFunction_NotEqual;
        case CompareFunction::GreaterEqual:
            return WGPUCompareFunction_GreaterEqual;
        case CompareFunction::Always:
            return WGPUCompareFunction_Always;
        }
        UNREACHABLE;
    }

    auto ToWgpu(BlendOperation op) -> WGPUBlendOperation
    {
        switch (op) {
        case BlendOperation::Add:
            return WGPUBlendOperation_Add;
        case BlendOperation::Subtract:
            return WGPUBlendOperation_Add;
        case BlendOperation::ReverseSubtract:
            return WGPUBlendOperation_Add;
        case BlendOperation::Min:
            return WGPUBlendOperation_Add;
        case BlendOperation::Max:
            return WGPUBlendOperation_Add;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(BlendFactor factor) -> WGPUBlendFactor
    {
        switch (factor) {
        case BlendFactor::Zero:
            return WGPUBlendFactor_Zero;
        case BlendFactor::One:
            return WGPUBlendFactor_One;
        case BlendFactor::Src:
            return WGPUBlendFactor_Src;
        case BlendFactor::OneMinusSrc:
            return WGPUBlendFactor_OneMinusSrc;
        case BlendFactor::SrcAlpha:
            return WGPUBlendFactor_SrcAlpha;
        case BlendFactor::OneMinusSrcAlpha:
            return WGPUBlendFactor_OneMinusSrcAlpha;
        case BlendFactor::Dst:
            return WGPUBlendFactor_Dst;
        case BlendFactor::OneMinusDst:
            return WGPUBlendFactor_OneMinusDst;
        case BlendFactor::DstAlpha:
            return WGPUBlendFactor_DstAlpha;
        case BlendFactor::OneMinusDstAlpha:
            return WGPUBlendFactor_OneMinusDstAlpha;
        case BlendFactor::SrcAlphaSaturated:
            return WGPUBlendFactor_SrcAlphaSaturated;
        case BlendFactor::Constant:
            return WGPUBlendFactor_Constant;
        case BlendFactor::OneMinusConstant:
            return WGPUBlendFactor_OneMinusConstant;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(StencilOperation op) -> WGPUStencilOperation
    {
        switch (op) {
        case StencilOperation::Keep:
            return WGPUStencilOperation_Keep;
        case StencilOperation::Zero:
            return WGPUStencilOperation_Zero;
        case StencilOperation::Replace:
            return WGPUStencilOperation_Replace;
        case StencilOperation::Invert:
            return WGPUStencilOperation_Invert;
        case StencilOperation::IncrementClamp:
            return WGPUStencilOperation_IncrementClamp;
        case StencilOperation::DecrementClamp:
            return WGPUStencilOperation_DecrementClamp;
        case StencilOperation::IncrementWrap:
            return WGPUStencilOperation_IncrementWrap;
        case StencilOperation::DecrementWrap:
            return WGPUStencilOperation_DecrementWrap;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto FromWgpu(WGPUErrorType type) -> ErrorType
    {
        switch (type) {
            using enum ErrorType;
        case WGPUErrorType_NoError:
            return NoError;
        case WGPUErrorType_Validation:
            return Validation;
        case WGPUErrorType_OutOfMemory:
            return OutOfMemory;
        case WGPUErrorType_Internal:
            return Internal;
        case WGPUErrorType_Unknown:
            return Unknown;
        case WGPUErrorType_DeviceLost:
            return DeviceLost;
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ToWgpu(TextureFormat format) -> WGPUTextureFormat
    {
        switch (format) {
        case TextureFormat::Undefined:
            return WGPUTextureFormat_Undefined;
        case TextureFormat::R8Unorm:
            return WGPUTextureFormat_R8Unorm;
        case TextureFormat::R8Snorm:
            return WGPUTextureFormat_R8Snorm;
        case TextureFormat::R8Uint:
            return WGPUTextureFormat_R8Uint;
        case TextureFormat::R8Sint:
            return WGPUTextureFormat_R8Sint;
        case TextureFormat::R16Uint:
            return WGPUTextureFormat_R16Uint;
        case TextureFormat::R16Sint:
            return WGPUTextureFormat_R16Sint;
        case TextureFormat::R16Float:
            return WGPUTextureFormat_R16Float;
        case TextureFormat::RG8Unorm:
            return WGPUTextureFormat_RG8Unorm;
        case TextureFormat::RG8Snorm:
            return WGPUTextureFormat_RG8Snorm;
        case TextureFormat::RG8Uint:
            return WGPUTextureFormat_RG8Uint;
        case TextureFormat::RG8Sint:
            return WGPUTextureFormat_RG8Sint;
        case TextureFormat::R32Float:
            return WGPUTextureFormat_R32Float;
        case TextureFormat::R32Uint:
            return WGPUTextureFormat_R32Uint;
        case TextureFormat::R32Sint:
            return WGPUTextureFormat_R32Sint;
        case TextureFormat::RG16Uint:
            return WGPUTextureFormat_RG16Uint;
        case TextureFormat::RG16Sint:
            return WGPUTextureFormat_RG16Sint;
        case TextureFormat::RG16Float:
            return WGPUTextureFormat_RG16Float;
        case TextureFormat::RGBA8Unorm:
            return WGPUTextureFormat_RGBA8Unorm;
        case TextureFormat::RGBA8UnormSrgb:
            return WGPUTextureFormat_RGBA8UnormSrgb;
        case TextureFormat::RGBA8Snorm:
            return WGPUTextureFormat_RGBA8Snorm;
        case TextureFormat::RGBA8Uint:
            return WGPUTextureFormat_RGBA8Uint;
        case TextureFormat::RGBA8Sint:
            return WGPUTextureFormat_RGBA8Sint;
        case TextureFormat::BGRA8Unorm:
            return WGPUTextureFormat_BGRA8Unorm;
        case TextureFormat::BGRA8UnormSrgb:
            return WGPUTextureFormat_BGRA8UnormSrgb;
        case TextureFormat::RGB10A2Uint:
            return WGPUTextureFormat_RGB10A2Uint;
        case TextureFormat::RGB10A2Unorm:
            return WGPUTextureFormat_RGB10A2Unorm;
        case TextureFormat::RG11B10Ufloat:
            return WGPUTextureFormat_RG11B10Ufloat;
        case TextureFormat::RGB9E5Ufloat:
            return WGPUTextureFormat_RGB9E5Ufloat;
        case TextureFormat::RG32Float:
            return WGPUTextureFormat_RG32Float;
        case TextureFormat::RG32Uint:
            return WGPUTextureFormat_RG32Uint;
        case TextureFormat::RG32Sint:
            return WGPUTextureFormat_RG32Sint;
        case TextureFormat::RGBA16Uint:
            return WGPUTextureFormat_RGBA16Uint;
        case TextureFormat::RGBA16Sint:
            return WGPUTextureFormat_RGBA16Sint;
        case TextureFormat::RGBA16Float:
            return WGPUTextureFormat_RGBA16Float;
        case TextureFormat::RGBA32Float:
            return WGPUTextureFormat_RGBA32Float;
        case TextureFormat::RGBA32Uint:
            return WGPUTextureFormat_RGBA32Uint;
        case TextureFormat::RGBA32Sint:
            return WGPUTextureFormat_RGBA32Sint;
        case TextureFormat::Stencil8:
            return WGPUTextureFormat_Stencil8;
        case TextureFormat::Depth16Unorm:
            return WGPUTextureFormat_Depth16Unorm;
        case TextureFormat::Depth24Plus:
            return WGPUTextureFormat_Depth24Plus;
        case TextureFormat::Depth24PlusStencil8:
            return WGPUTextureFormat_Depth24PlusStencil8;
        case TextureFormat::Depth32Float:
            return WGPUTextureFormat_Depth32Float;
        case TextureFormat::Depth32FloatStencil8:
            return WGPUTextureFormat_Depth32FloatStencil8;
        case TextureFormat::BC1RGBAUnorm:
            return WGPUTextureFormat_BC1RGBAUnorm;
        case TextureFormat::BC1RGBAUnormSrgb:
            return WGPUTextureFormat_BC1RGBAUnormSrgb;
        case TextureFormat::BC2RGBAUnorm:
            return WGPUTextureFormat_BC2RGBAUnorm;
        case TextureFormat::BC2RGBAUnormSrgb:
            return WGPUTextureFormat_BC2RGBAUnormSrgb;
        case TextureFormat::BC3RGBAUnorm:
            return WGPUTextureFormat_BC3RGBAUnorm;
        case TextureFormat::BC3RGBAUnormSrgb:
            return WGPUTextureFormat_BC3RGBAUnormSrgb;
        case TextureFormat::BC4RUnorm:
            return WGPUTextureFormat_BC4RUnorm;
        case TextureFormat::BC4RSnorm:
            return WGPUTextureFormat_BC4RSnorm;
        case TextureFormat::BC5RGUnorm:
            return WGPUTextureFormat_BC5RGUnorm;
        case TextureFormat::BC5RGSnorm:
            return WGPUTextureFormat_BC5RGSnorm;
        case TextureFormat::BC6HRGBUfloat:
            return WGPUTextureFormat_BC6HRGBUfloat;
        case TextureFormat::BC6HRGBFloat:
            return WGPUTextureFormat_BC6HRGBFloat;
        case TextureFormat::BC7RGBAUnorm:
            return WGPUTextureFormat_BC7RGBAUnorm;
        case TextureFormat::BC7RGBAUnormSrgb:
            return WGPUTextureFormat_BC7RGBAUnormSrgb;
        case TextureFormat::ETC2RGB8Unorm:
            return WGPUTextureFormat_ETC2RGB8Unorm;
        case TextureFormat::ETC2RGB8UnormSrgb:
            return WGPUTextureFormat_ETC2RGB8UnormSrgb;
        case TextureFormat::ETC2RGB8A1Unorm:
            return WGPUTextureFormat_ETC2RGB8A1Unorm;
        case TextureFormat::ETC2RGB8A1UnormSrgb:
            return WGPUTextureFormat_ETC2RGB8A1UnormSrgb;
        case TextureFormat::ETC2RGBA8Unorm:
            return WGPUTextureFormat_ETC2RGBA8Unorm;
        case TextureFormat::ETC2RGBA8UnormSrgb:
            return WGPUTextureFormat_ETC2RGBA8UnormSrgb;
        case TextureFormat::EACR11Unorm:
            return WGPUTextureFormat_EACR11Unorm;
        case TextureFormat::EACR11Snorm:
            return WGPUTextureFormat_EACR11Snorm;
        case TextureFormat::EACRG11Unorm:
            return WGPUTextureFormat_EACRG11Unorm;
        case TextureFormat::EACRG11Snorm:
            return WGPUTextureFormat_EACRG11Snorm;
        case TextureFormat::ASTC4x4Unorm:
            return WGPUTextureFormat_ASTC4x4Unorm;
        case TextureFormat::ASTC4x4UnormSrgb:
            return WGPUTextureFormat_ASTC4x4UnormSrgb;
        case TextureFormat::ASTC5x4Unorm:
            return WGPUTextureFormat_ASTC5x4Unorm;
        case TextureFormat::ASTC5x4UnormSrgb:
            return WGPUTextureFormat_ASTC5x4UnormSrgb;
        case TextureFormat::ASTC5x5Unorm:
            return WGPUTextureFormat_ASTC5x5Unorm;
        case TextureFormat::ASTC5x5UnormSrgb:
            return WGPUTextureFormat_ASTC5x5UnormSrgb;
        case TextureFormat::ASTC6x5Unorm:
            return WGPUTextureFormat_ASTC6x5Unorm;
        case TextureFormat::ASTC6x5UnormSrgb:
            return WGPUTextureFormat_ASTC6x5UnormSrgb;
        case TextureFormat::ASTC6x6Unorm:
            return WGPUTextureFormat_ASTC6x6Unorm;
        case TextureFormat::ASTC6x6UnormSrgb:
            return WGPUTextureFormat_ASTC6x6UnormSrgb;
        case TextureFormat::ASTC8x5Unorm:
            return WGPUTextureFormat_ASTC8x5Unorm;
        case TextureFormat::ASTC8x5UnormSrgb:
            return WGPUTextureFormat_ASTC8x5UnormSrgb;
        case TextureFormat::ASTC8x6Unorm:
            return WGPUTextureFormat_ASTC8x6Unorm;
        case TextureFormat::ASTC8x6UnormSrgb:
            return WGPUTextureFormat_ASTC8x6UnormSrgb;
        case TextureFormat::ASTC8x8Unorm:
            return WGPUTextureFormat_ASTC8x8Unorm;
        case TextureFormat::ASTC8x8UnormSrgb:
            return WGPUTextureFormat_ASTC8x8UnormSrgb;
        case TextureFormat::ASTC10x5Unorm:
            return WGPUTextureFormat_ASTC10x5Unorm;
        case TextureFormat::ASTC10x5UnormSrgb:
            return WGPUTextureFormat_ASTC10x5UnormSrgb;
        case TextureFormat::ASTC10x6Unorm:
            return WGPUTextureFormat_ASTC10x6Unorm;
        case TextureFormat::ASTC10x6UnormSrgb:
            return WGPUTextureFormat_ASTC10x6UnormSrgb;
        case TextureFormat::ASTC10x8Unorm:
            return WGPUTextureFormat_ASTC10x8Unorm;
        case TextureFormat::ASTC10x8UnormSrgb:
            return WGPUTextureFormat_ASTC10x8UnormSrgb;
        case TextureFormat::ASTC10x10Unorm:
            return WGPUTextureFormat_ASTC10x10Unorm;
        case TextureFormat::ASTC10x10UnormSrgb:
            return WGPUTextureFormat_ASTC10x10UnormSrgb;
        case TextureFormat::ASTC12x10Unorm:
            return WGPUTextureFormat_ASTC12x10Unorm;
        case TextureFormat::ASTC12x10UnormSrgb:
            return WGPUTextureFormat_ASTC12x10UnormSrgb;
        case TextureFormat::ASTC12x12Unorm:
            return WGPUTextureFormat_ASTC12x12Unorm;
        case TextureFormat::ASTC12x12UnormSrgb:
            return WGPUTextureFormat_ASTC12x12UnormSrgb;
        case TextureFormat::Force32:
            return WGPUTextureFormat_Force32;
        }
        UNREACHABLE;
    }

    auto FromWgpu(WGPUTextureFormat format) -> TextureFormat
    {
        switch (format) {
            using enum TextureFormat;
        case WGPUTextureFormat_Undefined:
            return Undefined;
        case WGPUTextureFormat_R8Unorm:
            return R8Unorm;
        case WGPUTextureFormat_R8Snorm:
            return R8Snorm;
        case WGPUTextureFormat_R8Uint:
            return R8Uint;
        case WGPUTextureFormat_R8Sint:
            return R8Sint;
        case WGPUTextureFormat_R16Uint:
            return R16Uint;
        case WGPUTextureFormat_R16Sint:
            return R16Sint;
        case WGPUTextureFormat_R16Float:
            return R16Float;
        case WGPUTextureFormat_RG8Unorm:
            return RG8Unorm;
        case WGPUTextureFormat_RG8Snorm:
            return RG8Snorm;
        case WGPUTextureFormat_RG8Uint:
            return RG8Uint;
        case WGPUTextureFormat_RG8Sint:
            return RG8Sint;
        case WGPUTextureFormat_R32Float:
            return R32Float;
        case WGPUTextureFormat_R32Uint:
            return R32Uint;
        case WGPUTextureFormat_R32Sint:
            return R32Sint;
        case WGPUTextureFormat_RG16Uint:
            return RG16Uint;
        case WGPUTextureFormat_RG16Sint:
            return RG16Sint;
        case WGPUTextureFormat_RG16Float:
            return RG16Float;
        case WGPUTextureFormat_RGBA8Unorm:
            return RGBA8Unorm;
        case WGPUTextureFormat_RGBA8UnormSrgb:
            return RGBA8UnormSrgb;
        case WGPUTextureFormat_RGBA8Snorm:
            return RGBA8Snorm;
        case WGPUTextureFormat_RGBA8Uint:
            return RGBA8Uint;
        case WGPUTextureFormat_RGBA8Sint:
            return RGBA8Sint;
        case WGPUTextureFormat_BGRA8Unorm:
            return BGRA8Unorm;
        case WGPUTextureFormat_BGRA8UnormSrgb:
            return BGRA8UnormSrgb;
        case WGPUTextureFormat_RGB10A2Uint:
            return RGB10A2Uint;
        case WGPUTextureFormat_RGB10A2Unorm:
            return RGB10A2Unorm;
        case WGPUTextureFormat_RG11B10Ufloat:
            return RG11B10Ufloat;
        case WGPUTextureFormat_RGB9E5Ufloat:
            return RGB9E5Ufloat;
        case WGPUTextureFormat_RG32Float:
            return RG32Float;
        case WGPUTextureFormat_RG32Uint:
            return RG32Uint;
        case WGPUTextureFormat_RG32Sint:
            return RG32Sint;
        case WGPUTextureFormat_RGBA16Uint:
            return RGBA16Uint;
        case WGPUTextureFormat_RGBA16Sint:
            return RGBA16Sint;
        case WGPUTextureFormat_RGBA16Float:
            return RGBA16Float;
        case WGPUTextureFormat_RGBA32Float:
            return RGBA32Float;
        case WGPUTextureFormat_RGBA32Uint:
            return RGBA32Uint;
        case WGPUTextureFormat_RGBA32Sint:
            return RGBA32Sint;
        case WGPUTextureFormat_Stencil8:
            return Stencil8;
        case WGPUTextureFormat_Depth16Unorm:
            return Depth16Unorm;
        case WGPUTextureFormat_Depth24Plus:
            return Depth24Plus;
        case WGPUTextureFormat_Depth24PlusStencil8:
            return Depth24PlusStencil8;
        case WGPUTextureFormat_Depth32Float:
            return Depth32Float;
        case WGPUTextureFormat_Depth32FloatStencil8:
            return Depth32FloatStencil8;
        case WGPUTextureFormat_BC1RGBAUnorm:
            return BC1RGBAUnorm;
        case WGPUTextureFormat_BC1RGBAUnormSrgb:
            return BC1RGBAUnormSrgb;
        case WGPUTextureFormat_BC2RGBAUnorm:
            return BC2RGBAUnorm;
        case WGPUTextureFormat_BC2RGBAUnormSrgb:
            return BC2RGBAUnormSrgb;
        case WGPUTextureFormat_BC3RGBAUnorm:
            return BC3RGBAUnorm;
        case WGPUTextureFormat_BC3RGBAUnormSrgb:
            return BC3RGBAUnormSrgb;
        case WGPUTextureFormat_BC4RUnorm:
            return BC4RUnorm;
        case WGPUTextureFormat_BC4RSnorm:
            return BC4RSnorm;
        case WGPUTextureFormat_BC5RGUnorm:
            return BC5RGUnorm;
        case WGPUTextureFormat_BC5RGSnorm:
            return BC5RGSnorm;
        case WGPUTextureFormat_BC6HRGBUfloat:
            return BC6HRGBUfloat;
        case WGPUTextureFormat_BC6HRGBFloat:
            return BC6HRGBFloat;
        case WGPUTextureFormat_BC7RGBAUnorm:
            return BC7RGBAUnorm;
        case WGPUTextureFormat_BC7RGBAUnormSrgb:
            return BC7RGBAUnormSrgb;
        case WGPUTextureFormat_ETC2RGB8Unorm:
            return ETC2RGB8Unorm;
        case WGPUTextureFormat_ETC2RGB8UnormSrgb:
            return ETC2RGB8UnormSrgb;
        case WGPUTextureFormat_ETC2RGB8A1Unorm:
            return ETC2RGB8A1Unorm;
        case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
            return ETC2RGB8A1UnormSrgb;
        case WGPUTextureFormat_ETC2RGBA8Unorm:
            return ETC2RGBA8Unorm;
        case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
            return ETC2RGBA8UnormSrgb;
        case WGPUTextureFormat_EACR11Unorm:
            return EACR11Unorm;
        case WGPUTextureFormat_EACR11Snorm:
            return EACR11Snorm;
        case WGPUTextureFormat_EACRG11Unorm:
            return EACRG11Unorm;
        case WGPUTextureFormat_EACRG11Snorm:
            return EACRG11Snorm;
        case WGPUTextureFormat_ASTC4x4Unorm:
            return ASTC4x4Unorm;
        case WGPUTextureFormat_ASTC4x4UnormSrgb:
            return ASTC4x4UnormSrgb;
        case WGPUTextureFormat_ASTC5x4Unorm:
            return ASTC5x4Unorm;
        case WGPUTextureFormat_ASTC5x4UnormSrgb:
            return ASTC5x4UnormSrgb;
        case WGPUTextureFormat_ASTC5x5Unorm:
            return ASTC5x5Unorm;
        case WGPUTextureFormat_ASTC5x5UnormSrgb:
            return ASTC5x5UnormSrgb;
        case WGPUTextureFormat_ASTC6x5Unorm:
            return ASTC6x5Unorm;
        case WGPUTextureFormat_ASTC6x5UnormSrgb:
            return ASTC6x5UnormSrgb;
        case WGPUTextureFormat_ASTC6x6Unorm:
            return ASTC6x6Unorm;
        case WGPUTextureFormat_ASTC6x6UnormSrgb:
            return ASTC6x6UnormSrgb;
        case WGPUTextureFormat_ASTC8x5Unorm:
            return ASTC8x5Unorm;
        case WGPUTextureFormat_ASTC8x5UnormSrgb:
            return ASTC8x5UnormSrgb;
        case WGPUTextureFormat_ASTC8x6Unorm:
            return ASTC8x6Unorm;
        case WGPUTextureFormat_ASTC8x6UnormSrgb:
            return ASTC8x6UnormSrgb;
        case WGPUTextureFormat_ASTC8x8Unorm:
            return ASTC8x8Unorm;
        case WGPUTextureFormat_ASTC8x8UnormSrgb:
            return ASTC8x8UnormSrgb;
        case WGPUTextureFormat_ASTC10x5Unorm:
            return ASTC10x5Unorm;
        case WGPUTextureFormat_ASTC10x5UnormSrgb:
            return ASTC10x5UnormSrgb;
        case WGPUTextureFormat_ASTC10x6Unorm:
            return ASTC10x6Unorm;
        case WGPUTextureFormat_ASTC10x6UnormSrgb:
            return ASTC10x6UnormSrgb;
        case WGPUTextureFormat_ASTC10x8Unorm:
            return ASTC10x8Unorm;
        case WGPUTextureFormat_ASTC10x8UnormSrgb:
            return ASTC10x8UnormSrgb;
        case WGPUTextureFormat_ASTC10x10Unorm:
            return ASTC10x10Unorm;
        case WGPUTextureFormat_ASTC10x10UnormSrgb:
            return ASTC10x10UnormSrgb;
        case WGPUTextureFormat_ASTC12x10Unorm:
            return ASTC12x10Unorm;
        case WGPUTextureFormat_ASTC12x10UnormSrgb:
            return ASTC12x10UnormSrgb;
        case WGPUTextureFormat_ASTC12x12Unorm:
            return ASTC12x12Unorm;
        case WGPUTextureFormat_ASTC12x12UnormSrgb:
            return ASTC12x12UnormSrgb;
        case WGPUTextureFormat_Force32:
            return Force32;
        }
    }
}