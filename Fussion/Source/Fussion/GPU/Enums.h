#pragma once
#include <Fussion/Core/BitFlags.h>
#include <Fussion/Core/Core.h>

#define WGPU_LOCAL

namespace Fussion::GPU {
    enum class BackendRenderer {
        Vulkan,
        DX11,
        DX12,
        OpenGL,
    };

    enum class Feature {
        TimestampQuery,
        PipelineStatistics,
        Float32Filterable,

#ifdef WGPU_LOCAL
        SpirVPassthrough,
#endif
    };

    // enum class QueryType {
    //     Occlusion,
    //     Timestamp,
    // };

    enum class PipelineStatisticName {
        VertexShaderInvocations,
        ClipperInvocations,
        ClipperPrimitivesOut,
        FragmentShaderInvocations,
        ComputeShaderInvocations,
    };

    BITFLAGS(PipelineStatisticName)

    enum class BufferUsage {
        None = 1 << 0,
        MapRead = 1 << 1,
        MapWrite = 1 << 2,
        CopySrc = 1 << 3,
        CopyDst = 1 << 4,
        Index = 1 << 5,
        Vertex = 1 << 6,
        Uniform = 1 << 7,
        Storage = 1 << 8,
        Indirect = 1 << 9,
        QueryResolve = 1 << 10,
        Force32 = 1 << 11,
    };

    BITFLAGS(BufferUsage)

    enum class MapState {
        Unmapped,
        Pending,
        Mapped,
    };

    enum class MapMode {
        None = 1 << 0,
        Read = 1 << 1,
        Write = 1 << 2,
    };

    BITFLAGS(MapMode)

    enum class TextureUsage {
        None = 1 << 0,
        CopySrc = 1 << 1,
        CopyDst = 1 << 2,
        TextureBinding = 1 << 3,
        StorageBinding = 1 << 4,
        RenderAttachment = 1 << 5,
    };

    BITFLAGS(TextureUsage);

    enum class ShaderStage {
        None = 0,
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
    };

    BITFLAGS(ShaderStage)

    enum class DevicePower {
        Undefined,
        LowPower,
        HighPerformance,
    };

    enum class PresentMode {
        Fifo,
        FifoRelaxed,
        Immediate,
        Mailbox,
    };

    enum class AddressMode {
        ClampToEdge,
        Repeat,
        MirrorRepeat,
        ClampToBorder,
    };

    enum class FilterMode {
        Nearest = 0,
        Linear = 1,
    };

    enum class ErrorType {
        NoError,
        Validation,
        OutOfMemory,
        Internal,
        Unknown,
        DeviceLost,
    };

    enum class TextureDimension {
        D1,
        D2,
        D3,
    };

    enum class TextureViewDimension {
        Undefined,
        D1,
        D2,
        D2_Array,
        Cube,
        CubeArray,
        D3,
    };

    enum class TextureAspect {
        All,
        StencilOnly,
        DepthOnly,
    };

    enum class ElementType {
        Int,
        Int2,
        Int3,
        Int4,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
    };

    inline u32 element_type_count(ElementType type)
    {
        switch (type) {
            using enum ElementType;
        case Int:
        case Float:
            return 1;
        case Int2:
        case Float2:
            return 2;
        case Int3:
        case Float3:
            return 3;
        case Int4:
        case Float4:
            return 4;
        case Mat3:
            return 3 * 3;
        case Mat4:
            return 4 * 4;
        }
        UNREACHABLE;
    }

    enum class VertexStepMode {
        Vertex,
        Instance,
    };

    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    };

    enum class IndexFormat {
        Undefined,
        U16,
        U32,
    };

    enum class FrontFace {
        Ccw,
        Cw,
    };

    enum class Face {
        None,
        Front,
        Back,
    };

    enum class CompareFunction {
        Undefined,
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    enum class StencilOperation {
        Keep = 0,
        Zero = 1,
        Replace = 2,
        Invert = 3,
        IncrementClamp = 4,
        DecrementClamp = 5,
        IncrementWrap = 6,
        DecrementWrap = 7,
    };

    enum class BlendFactor {
        Zero = 0,
        One = 1,
        Src = 2,
        OneMinusSrc = 3,
        SrcAlpha = 4,
        OneMinusSrcAlpha = 5,
        Dst = 6,
        OneMinusDst = 7,
        DstAlpha = 8,
        OneMinusDstAlpha = 9,
        SrcAlphaSaturated = 10,
        Constant = 11,
        OneMinusConstant = 12,
    };

    enum class BlendOperation {
        Add = 0,
        Subtract = 1,
        ReverseSubtract = 2,
        Min = 3,
        Max = 4,
    };

    enum class ColorWrite {
        Red = 1 << 0,
        Green = 1 << 1,
        Blue = 1 << 2,
        Alpha = 1 << 3,

        Color = Red | Green | Blue,
        All = Color | Alpha,
    };

    BITFLAGS(ColorWrite)

    /// Not yet used.
    enum class PolygonMode {
        Fill,
        Line,
        Point,
    };

    enum class LoadOp {
        Undefined,
        Clear,
        Load,
    };

    enum class StoreOp {
        Undefined,
        Store,
        Discard,
    };

    enum class SamplerBindingType {
        Filtering,
        NonFiltering,
        Comparison,
    };

    enum class StorageAccess {
        WriteOnly,
        ReadOnly,
        ReadWrite,
    };

    enum class TextureFormat {
        Undefined,
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        R32Float,
        R32Uint,
        R32Sint,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        RGB10A2Uint,
        RGB10A2Unorm,
        RG11B10Ufloat,
        RGB9E5Ufloat,
        RG32Float,
        RG32Uint,
        RG32Sint,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        RGBA32Float,
        RGBA32Uint,
        RGBA32Sint,
        Stencil8,
        Depth16Unorm,
        Depth24Plus,
        Depth24PlusStencil8,
        Depth32Float,
        Depth32FloatStencil8,
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBUfloat,
        BC6HRGBFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        ETC2RGB8Unorm,
        ETC2RGB8UnormSrgb,
        ETC2RGB8A1Unorm,
        ETC2RGB8A1UnormSrgb,
        ETC2RGBA8Unorm,
        ETC2RGBA8UnormSrgb,
        EACR11Unorm,
        EACR11Snorm,
        EACRG11Unorm,
        EACRG11Snorm,
        ASTC4x4Unorm,
        ASTC4x4UnormSrgb,
        ASTC5x4Unorm,
        ASTC5x4UnormSrgb,
        ASTC5x5Unorm,
        ASTC5x5UnormSrgb,
        ASTC6x5Unorm,
        ASTC6x5UnormSrgb,
        ASTC6x6Unorm,
        ASTC6x6UnormSrgb,
        ASTC8x5Unorm,
        ASTC8x5UnormSrgb,
        ASTC8x6Unorm,
        ASTC8x6UnormSrgb,
        ASTC8x8Unorm,
        ASTC8x8UnormSrgb,
        ASTC10x5Unorm,
        ASTC10x5UnormSrgb,
        ASTC10x6Unorm,
        ASTC10x6UnormSrgb,
        ASTC10x8Unorm,
        ASTC10x8UnormSrgb,
        ASTC10x10Unorm,
        ASTC10x10UnormSrgb,
        ASTC12x10Unorm,
        ASTC12x10UnormSrgb,
        ASTC12x12Unorm,
        ASTC12x12UnormSrgb,
        Force32
    };

}
