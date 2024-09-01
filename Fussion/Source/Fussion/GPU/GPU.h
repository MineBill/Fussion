#pragma once
#include "Fussion/Core/Ref.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector3.h"
#include <Fussion/Core/Maybe.h>

#include <Fussion/Core/Types.h>
#include <Fussion/GPU/Enums.h>
#include <Fussion/Window.h>

#include <functional>
#include <span>
#include <variant>
#include <concepts>

template<Fussion::ScalarType T>
struct Range {
    T Start{};
    T End{};

    T Count() const { return End - Start; }
};

namespace Fussion::GPU {
    using HandleT = void*;

    using ErrorFn = std::function<void(ErrorType, std::string_view)>;

    template<typename SpecT>
    struct GPUHandle {
        HandleT Handle{};
        SpecT Spec{};

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle): Handle(handle) {}
        GPUHandle(HandleT handle, SpecT const& spec): Handle(handle), Spec(spec) {}

        template<typename T>
        auto As() const -> T
        {
            return CAST(T, Handle);
        }

        virtual void Release() = 0;
    };

    template<>
    struct GPUHandle<void> {
        HandleT Handle{};

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle): Handle(handle) {}

        template<typename T>
        auto As() const -> T
        {
            return CAST(T, Handle);
        }

        virtual void Release() = 0;
    };

    struct BufferSpec {
        const char* Label{};
        BufferUsageFlags Usage{};
        u64 Size{};
        bool Mapped{ false };
    };

    struct BufferSlice;

    struct Buffer final : GPUHandle<BufferSpec> {
        using GPUHandle::GPUHandle;

        auto GetSize() const -> u64;
        auto GetSlice(u32 start, u32 size) -> BufferSlice;

        virtual void Release() override;
    };

    struct BufferSlice {
        Buffer BackingBuffer;
        u32 Start{};
        u32 Size{};

        BufferSlice(Buffer const& buffer, u32 start, u32 size);
        BufferSlice(Buffer const& buffer);
    };

    struct TextureSpec {
        Maybe<const char*> Label{};
        TextureUsageFlags Usage{};
        TextureDimension Dimension{};
        Vector3 Size{};
        TextureFormat Format{};
        u32 MipLevelCount{ 1 };
        u32 SampleCount{ 1 };
    };

    struct TextureViewSpec {
        Maybe<const char*> Label{};
        TextureUsageFlags Usage{};
        TextureViewDimension Dimension{};
        TextureFormat Format{};

        u32 BaseMipLevel{ 0 };
        u32 MipLevelCount{ 1 };
        u32 BaseArrayLayer{ 0 };
        u32 ArrayLayerCount{ 1 };

        TextureAspect Aspect{};
    };

    struct TextureView final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct Texture final : GPUHandle<TextureSpec> {
        using GPUHandle::GPUHandle;

        TextureView View{};

        void InitializeView();
        TextureView CreateView(TextureViewSpec const& spec) const;

        virtual void Release() override;
    };

    struct Sampler {
        HandleT Handle{};
    };

    namespace BufferBindingType {
        struct Uniform {};

        struct Storage {
            bool ReadOnly{};
        };

        using Type = std::variant<Uniform, Storage>;
    };

    namespace TextureSampleType {
        struct Float {
            bool Filterable{};
        };

        struct Depth {};

        struct SInt {};

        struct UInt {};

        using Type = std::variant<Float, Depth, SInt, UInt>;
    };

    namespace BindingType {
        struct Buffer {
            BufferBindingType::Type Type{};
            bool HasDynamicOffset{};
            Maybe<u64> MinBindingSize{};
        };

        struct Sampler {
            SamplerBindingType Type{};
        };

        struct Texture {
            TextureSampleType::Type SampleType{};
            TextureViewDimension ViewDimension{};
            bool MultiSampled{};
        };

        struct StorageTexture {
            StorageAccess Access{};
            TextureFormat Format{};
            TextureViewDimension ViewDimension{};
        };

        struct AccelerationStructure {};

        using Type = std::variant<Buffer, Sampler, Texture, StorageTexture, AccelerationStructure>;
    };

    struct BindGroupLayoutEntry {
        u32 Binding{};
        ShaderStageFlags Visibility{};
        BindingType::Type Type{};
        Maybe<u32> Count{};
    };

    struct BindGroupLayoutSpec {
        Maybe<const char*> Label{};
        std::span<BindGroupLayoutEntry> Entries{};
    };

    struct BindGroupLayout {
        HandleT Handle{};
    };

    struct BufferBinding {
        Buffer Buffer{};
        u64 Offset{};
        u64 Size{};
    };

    using BindingResource = std::variant<BufferBinding, TextureView, Sampler>;

    struct BindGroupEntry {
        u32 Binding{};
        BindingResource Resource;
    };

    struct BindGroupSpec {
        /// Label for the bind group.
        Maybe<const char*> Label{};
        std::span<BindGroupEntry> Entries{};
    };

    /// A BindGroup represents the set of resources bound to the bindings
    /// described by a BindGroupLayout. It can be created with Device::CreateBindGroup.
    /// A BindGroup can be bound to a particular RenderPass with RenderPassEncoder::SetBindGroup,
    /// or to a ComputePass with ComputePassEncoder::SetBindGroup.
    struct BindGroup {
        HandleT Handle{};
    };

    struct PipelineLayoutSpec {
        Maybe<const char*> Label{};
        std::span<BindGroupLayout> BindGroupLayouts{};
    };

    struct PipelineLayout final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct ShaderModuleSpec {
        Maybe<const char*> Label{};
        std::string_view Source{};

        std::string_view VertexEntryPoint{};
        std::string_view FragmentEntryPoint{};
    };

    struct ShaderModule final : GPUHandle<ShaderModuleSpec> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    /// Vertex inputs (attributes) to shaders.
    struct VertexAttribute {
        /// Format of the input
        ElementType Type{};
        /// Byte offset of the start of the input
        u32 Offset{};
        /// Location for this input. Must match the location in the shader.
        u32 ShaderLocation{};
    };

    /// Describes how the vertex buffer is interpreted.
    /// For use in VertexState.
    struct VertexBufferLayout {
        /// The stride, in bytes, between elements of this buffer.
        u64 ArrayStride{};
        /// How often this vertex buffer is “stepped” forward.
        VertexStepMode StepMode{};
        /// The list of attributes which comprise a single vertex.
        std::vector<VertexAttribute> Attributes{};

        /// @param attributes The list of attributes which comprise a single vertex.
        static VertexBufferLayout Create(std::span<VertexAttribute> attributes)
        {
            VertexBufferLayout self;
            u32 offset = 0;
            for (auto& attr : attributes) {
                attr.Offset = offset;
                self.Attributes.push_back(attr);

                offset += ElementTypeCount(attr.Type) * 4_u32;
            }
            self.ArrayStride = offset;
            return self;
        }

    private:
        // Only allow creating layouts from the VertexAttributeLayout::Create static method.
        VertexBufferLayout() = default;
    };

    struct VertexState {
        // /// The compiled shader module for this stage.
        // ShaderModule Module{};
        // /// The name of the entry point in the compiled shader. There must be a function with this name in the shader.
        // std::string_view EntryPoint{};
        /// The format of any vertex buffers used with this pipeline.
        std::span<VertexBufferLayout> AttributeLayouts{};
    };

    struct PrimitiveState {
        PrimitiveTopology Topology{};
        Maybe<IndexFormat> StripIndexFormat{};
        FrontFace FrontFace{};
        Face Cull{};
    };

    struct StencilFaceState {
        CompareFunction Compare{};
        StencilOperation FailOp{};
        StencilOperation DepthFailOp{};
        StencilOperation PassOp{};
    };

    struct StencilState {
        StencilFaceState Front{};
        StencilFaceState Back{};
        u32 ReadMask{};
        u32 WriteMask{};
    };

    struct DepthBiasState {
        /// Constant depth biasing factor, in basic units of the depth format.
        s32 Constant{};

        /// Slope depth biasing factor.
        f32 SlopeScale{};

        /// Depth bias clamp value (absolute).
        f32 Clamp{};
    };

    struct DepthStencilState {
        TextureFormat Format{};
        bool DepthWriteEnabled{};
        CompareFunction DepthCompare{};
        StencilState Stencil{};
        DepthBiasState Bias{};
    };

    struct BlendComponent {
        BlendFactor SrcFactor{};
        BlendFactor DstFactor{};
        BlendOperation Operation{};
    };

    struct BlendState {
        BlendComponent Color{}, Alpha{};
    };

    struct ColorTargetState {
        /// The TextureFormat of the image that this pipeline will render to.
        /// Must match the format of the corresponding color attachment in CommandEncoder::BeginRendering
        TextureFormat Format{};
        /// The blending that is used for this pipeline.
        Maybe<BlendState> Blend{};
        /// Mask which enables/disables writes to different color/alpha channel.
        ColorWriteFlags WriteMask{};
    };

    struct FragmentStage {
        std::span<ColorTargetState> Targets{};
    };

    struct MultiSampleState {
        /// The number of samples calculated per pixel (for MSAA).
        /// For non-multi-sampled textures, this should be 1
        u32 Count{ 1 };
        /// Bitmask that restricts the samples of a pixel modified by this pipeline.
        /// All samples can be enabled using the value !0
        u32 Mask{};
        /// When enabled, produces another sample mask per pixel based on the
        /// alpha output value, that is ANDed with the sample_mask and the
        /// primitive coverage to restrict the set of samples affected by a primitive.
        /// The implicit mask produced for alpha of zero is guaranteed to be zero,
        /// and for alpha of one is guaranteed to be all 1-s.
        bool AlphaToCoverageEnabled{ false };
    };

    struct RenderPipelineSpec {
        Maybe<const char*> Label{};
        Maybe<PipelineLayout> Layout{};
        VertexState Vertex{};
        PrimitiveState Primitive{};
        Maybe<DepthStencilState> DepthStencil{};
        MultiSampleState MultiSample{};
        FragmentStage Fragment{};
        // MutliView
        // Cache
    };

    struct RenderPipeline : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct RenderPassColorAttachment {
        TextureView View{};
        LoadOp LoadOp{};
        StoreOp StoreOp{};
        Color ClearColor{};
        f32 DepthClear{};
    };

    struct RenderPassSpec {
        Maybe<const char*> Label{};
        std::span<RenderPassColorAttachment> ColorAttachments{};
        Maybe<RenderPassColorAttachment> DepthStencilAttachment{};
    };

    struct RenderPassEncoder : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        /// Sets the viewport used during the rasterization stage to
        /// linearly map from normalized device coordinates to viewport coordinates.
        /// Subsequent draw calls will only draw within this region.
        /// If this method has not been called, the viewport defaults
        /// to the entire bounds of the render targets.
        void SetViewport(Vector2 const& origin, Vector2 const& size, f32 min_depth, f32 max_depth) const;

        void SetBindGroup(BindGroup group, u32 index) const;
        void SetVertexBuffer(u32 slot, BufferSlice const& slice) const;
        void SetPipeline(RenderPipeline const& pipeline) const;
        void Draw(Range<u32> vertices, Range<u32> instances) const;

        void End() const;
        virtual void Release() override;
    };

    struct CommandBuffer {
        HandleT Handle{};
    };

    struct CommandEncoder {
        HandleT Handle{};

        auto BeginRendering(RenderPassSpec const& spec) const -> RenderPassEncoder;
        auto Finish() -> CommandBuffer;

        void Release() const;
    };

    struct Device {
        HandleT Handle{};
        HandleT Queue{};

        explicit Device(HandleT handle);
        void Release() const;

        void SetErrorCallback(ErrorFn const& function);

        auto CreateBuffer(BufferSpec const& spec) const -> Buffer;
        auto CreateTexture(TextureSpec const& spec) const -> Texture;
        auto CreateCommandEncoder(const char* label = "") const -> CommandEncoder;
        auto CreateBindGroup(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup;
        auto CreateBindGroupLayout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout;

        auto CreateShaderModule(ShaderModuleSpec const& spec) const -> ShaderModule;
        auto CreatePipelineLayout(PipelineLayoutSpec const& spec) const -> PipelineLayout;
        auto CreateRenderPipeline(ShaderModule const& module, RenderPipelineSpec const& spec) const -> RenderPipeline;

        void SubmitCommandBuffer(CommandBuffer cmd) const;

        void WriteBuffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const;
        void WriteTexture(Texture const& texture, void const* data, size_t data_size, Vector2 const& origin, Vector2 const& size, u32 mip_level = 0) const;

    private:
        ErrorFn m_Function{};
    };

    struct AdapterOptions {
        DevicePower PowerPreference{ DevicePower::HighPerformance };
    };

    struct Adapter {
        HandleT Handle{};

        auto GetDevice() -> Device;

        void Release() const;
    };

    struct Surface {
        struct Config {
            PresentMode PresentMode{ PresentMode::Immediate };
            Vector2 Size{};
        };

        HandleT Handle{};
        TextureFormat Format{};

        void Release() const;

        void Configure(Device const& device, Adapter adapter, Config const& config);
        auto GetNextView() -> TextureView;
        void Present() const;
    };

    struct InstanceSpec {
        BackendRenderer Backend{ BackendRenderer::Vulkan };
    };

    struct Instance {
        HandleT Handle{};

        static Instance Create(InstanceSpec const& spec = {});

        void Release() const;

        auto GetAdapter(Surface surface, AdapterOptions const& opt = {}) const -> Adapter;
        auto GetSurface(Window const* window) const -> Surface;
    };


}
