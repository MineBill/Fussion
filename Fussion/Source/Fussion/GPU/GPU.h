#pragma once
#include "Fussion/Core/Ref.h"
#include "Fussion/Core/String.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Math/Vector3.h"
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Types.h>
#include <Fussion/GPU/Enums.h>
#include <Fussion/Window.h>

#include <functional>
#include <memory_resource>
#include <span>
#include <variant>

#define WGPU_DEV

// TODO: Add tests for this
template<Fussion::ScalarType T>
struct Range {
    T start {};
    T stop {};

    T count() const { return stop - start; }

    class Iterator {
        T m_number {}, m_start {}, m_end {};

    public:
        explicit Iterator(T start, T end, T number = 0)
            : m_number(number)
            , m_start(start)
            , m_end(end)
        {
        }

        Iterator& operator++()
        {
            m_number = m_end >= m_start ? m_number + 1 : m_number - 1;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(Iterator other) const { return m_number == other.m_number; }
        bool operator!=(Iterator other) const { return !(*this == other); }
        T const& operator*() const { return m_number; }
    };

    Iterator begin() { return Iterator(start, stop, start); }
    Iterator end() { return Iterator(start, stop, stop >= start ? stop + 1 : stop - 1); }
};

namespace Fussion::GPU {
    using HandleT = void*;

    using ErrorFn = std::function<void(ErrorType, std::string_view)>;

    struct Device;

    template<typename SpecT>
    struct GPUHandle {
        HandleT Handle {};
        SpecT Spec {};

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle)
            : Handle(handle)
        {
        }
        GPUHandle(HandleT handle, SpecT const& spec)
            : Handle(handle)
            , Spec(spec)
        {
        }

        template<typename T>
        auto As() const -> T
        {
            return CAST(T, Handle);
        }

        virtual void Release() = 0;
    };

    template<>
    struct GPUHandle<void> {
        HandleT Handle { nullptr };

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle)
            : Handle(handle)
        {
        }

        template<typename T>
        auto As() const -> T
        {
            return CAST(T, Handle);
        }

        virtual void Release() = 0;
    };

    struct RegistryReport {
        size_t NumAllocated;
        size_t NumKeptFromUser;
        size_t NumReleasedFromUser;
        size_t NumError;
        size_t ElementSize;
    };

    struct GlobalReport {
        RegistryReport Adapters;
        RegistryReport Devices;
        RegistryReport Queues;
        RegistryReport PipelineLayouts;
        RegistryReport ShaderModules;
        RegistryReport BindGroupLayouts;
        RegistryReport BindGroups;
        RegistryReport CommandBuffers;
        RegistryReport RenderBundles;
        RegistryReport RenderPipelines;
        RegistryReport ComputePipelines;
        RegistryReport QuerySets;
        RegistryReport Buffers;
        RegistryReport Textures;
        RegistryReport TextureViews;
        RegistryReport Samplers;
    };

    namespace QueryType {
        struct Occlusion { };

        struct Timestamp { };

        using Type = std::variant<Occlusion, PipelineStatisticNameFlags, Timestamp>;
    }

    struct QuerySetSpec {
        Maybe<String> Label {};
        QueryType::Type Type {};
        u32 Count {};
    };

    struct QuerySet final : GPUHandle<QuerySetSpec> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct SamplerSpec {
        Maybe<std::string_view> label {};
        AddressMode AddressModeU { AddressMode::Repeat };
        AddressMode AddressModeV { AddressMode::Repeat };
        AddressMode AddressModeW { AddressMode::Repeat };
        FilterMode MagFilter { FilterMode::Linear };
        FilterMode MinFilter { FilterMode::Linear };
        FilterMode MipMapFilter { FilterMode::Linear };
        f32 LodMinClamp { 0.0f };
        f32 LodMaxClamp { 32.0f };
        Maybe<CompareFunction> CompareFunc {};
        u16 AnisotropyClamp { 1_u16 };
    };

    struct Sampler final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct BufferSpec {
        Maybe<std::string_view> Label {};
        BufferUsageFlags Usage {};
        u64 Size {};
        bool Mapped { false };
    };

    struct BufferSlice;

    struct Buffer final : GPUHandle<BufferSpec> {
        using GPUHandle::GPUHandle;

        Buffer(HandleT handle, BufferSpec const& spec);
        auto Size() const -> u64;
        auto Slice(u32 start, u32 size) -> BufferSlice;
        auto Slice() -> BufferSlice;
        auto GetMapState() const -> MapState;

        void Unmap();

        virtual void Release() override;
        void ForceMapState(MapState state);

        friend BufferSlice;

    private:
        MapState m_CurrentMapState { MapState::Unmapped };
    };

    struct BufferSlice {
        using AsyncMapCallback = std::function<void()>;

        Buffer* BackingBuffer;
        u32 Start {};
        u32 Size {};

        BufferSlice(Buffer& buffer, u32 start, u32 size);
        BufferSlice(Buffer& buffer);

        /// Returns the slice as a mapped ranged.
        /// This assumes buffer was created with Mapped = true
        /// or that this call is a result from map of the buffer.
        auto MappedRange() -> void*;

        void MapAsync(MapModeFlags map_mode, AsyncMapCallback const& callback) const;
    };

    struct TextureSpec {
        Maybe<std::string_view> Label {};
        TextureUsageFlags Usage {};
        TextureDimension Dimension {};
        Vector3 Size {};
        TextureFormat Format {};
        // u32 MipLevelCount{ 1 };
        u32 SampleCount { 1 };
        TextureAspect Aspect {};

        bool GenerateMipMaps { false };
        bool InitializeView { true };
    };

    struct TextureViewSpec {
        Maybe<std::string_view> Label {};
        TextureUsageFlags Usage {};
        TextureViewDimension Dimension {};
        TextureFormat Format {};

        u32 BaseMipLevel { 0 };
        u32 MipLevelCount { 1 };
        u32 BaseArrayLayer { 0 };
        u32 ArrayLayerCount { 1 };

        TextureAspect Aspect {};
    };

    struct TextureView final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        operator HandleT() const
        {
            return Handle;
        }

        virtual void Release() override;
    };

    struct Texture final : GPUHandle<TextureSpec> {
        using GPUHandle::GPUHandle;

        u32 MipLevelCount {};
        TextureView View {};

        void InitializeView(u32 array_count = 1);
        void GenerateMipmaps(Device const& device);
        auto CreateView(TextureViewSpec const& spec) const -> TextureView;

        virtual void Release() override;
    };

    namespace BufferBindingType {
        struct Uniform { };

        struct Storage {
            bool ReadOnly {};
        };

        using Type = std::variant<Uniform, Storage>;
    };

    namespace TextureSampleType {
        struct Float {
            bool Filterable { true };
        };

        struct Depth { };

        struct SInt { };

        struct UInt { };

        using Type = std::variant<Float, Depth, SInt, UInt>;
    };

    namespace BindingType {
        struct Buffer {
            /// Sub-type of the buffer binding.
            BufferBindingType::Type Type {};
            /// Indicates that the binding has a dynamic offset.
            /// One offset must be passed to RenderPass::set_bind_group
            /// for each dynamic binding in increasing order of binding number.
            bool HasDynamicOffset {};
            /// The minimum size for a BufferBinding matching this entry, in bytes.
            /// If this is Some(size):
            ///     When calling create_bind_group, the resource at this bind
            ///     point must be a BindingResource::Buffer whose effective size is at least size.
            ///     When calling create_render_pipeline or create_compute_pipeline,
            ///     size must be at least the minimum buffer binding size for the
            ///     shader module global at this bind point: large enough to hold the
            ///     global’s value, along with one element of a trailing runtime-sized array, if present.
            /// If this is None:
            ///     Each draw or dispatch command checks that the buffer range at this bind point satisfies the minimum buffer binding size.
            Maybe<u64> MinBindingSize {};
        };

        struct Sampler {
            SamplerBindingType Type {};
        };

        struct Texture {
            TextureSampleType::Type SampleType {};
            TextureViewDimension ViewDimension {};
            bool MultiSampled { false };
        };

        struct StorageTexture {
            StorageAccess Access {};
            TextureFormat Format {};
            TextureViewDimension ViewDimension {};
        };

        struct AccelerationStructure { };

        using Type = std::variant<Buffer, Sampler, Texture, StorageTexture, AccelerationStructure>;
    };

    struct BindGroupLayoutEntry {
        u32 Binding {};
        ShaderStageFlags Visibility {};
        BindingType::Type Type {};
        Maybe<u32> Count {};
    };

    struct BindGroupLayoutSpec {
        /// Debug label of the render pass. This will show up in graphics debuggers for easy identification.
        Maybe<std::string_view> Label {};
        std::span<BindGroupLayoutEntry> Entries {};
    };

    struct BindGroupLayout {
        HandleT Handle {};
    };

    /// Binding for uniform or storage buffers.
    struct BufferBinding {
        /// The buffer to bind.
        Buffer TargetBuffer {};

        /// Base offset of the buffer, in bytes.
        ///
        /// If the has_dynamic_offset field of this buffer’s layout entry is true,
        /// the offset here will be added to the dynamic offset passed
        /// to RenderPass::set_bind_group or ComputePass::set_bind_group.
        ///
        /// If the buffer was created with BufferUsages::UNIFORM,
        /// then this offset must be a multiple of Limits::min_uniform_buffer_offset_alignment.
        ///
        /// If the buffer was created with BufferUsages::STORAGE,
        /// then this offset must be a multiple of Limits::min_storage_buffer_offset_alignment.
        u64 Offset {};
        /// Size of the binding in bytes, or None for using the rest of the buffer.
        u64 Size {};
    };

    using BindingResource = std::variant<BufferBinding, TextureView, Sampler>;

    struct BindGroupEntry {
        u32 Binding {};
        BindingResource Resource;
    };

    struct BindGroupSpec {
        /// Debug label of the render pass. This will show up in graphics debuggers for easy identification.
        Maybe<std::string_view> Label {};
        std::span<BindGroupEntry> Entries {};
    };

    /// A BindGroup represents the set of resources bound to the bindings
    /// described by a BindGroupLayout. It can be created with Device::create_bind_group().
    /// A BindGroup can be bound to a particular RenderPass with RenderPassEncoder::set_bind_group(),
    /// or to a ComputePass with ComputePassEncoder::set_bind_group.
    struct BindGroup final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct PipelineLayoutSpec {
        /// Debug label of the render pass. This will show up in graphics debuggers for easy identification.
        Maybe<std::string_view> Label {};
        std::span<BindGroupLayout> BindGroupLayouts {};
    };

    struct PipelineLayout final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    struct WGSLShader {
        std::string_view Source {};
    };

    struct SPIRVShader {
        std::span<u32> Binary {};
    };

    using ShaderType = std::variant<WGSLShader, SPIRVShader>;

    struct ShaderModuleSpec {
        Maybe<std::string_view> Label {};
        ShaderType Type {};

        std::string_view VertexEntryPoint {};
        std::string_view FragmentEntryPoint {};
    };

    struct SpirVShaderSpec {
        Maybe<std::string_view> Label {};
        std::span<u32> Data {};
    };

    struct ShaderModule final : GPUHandle<ShaderModuleSpec> {
        using GPUHandle::GPUHandle;

        virtual void Release() override;
    };

    /// Vertex inputs (attributes) to shaders.
    struct VertexAttribute {
        std::string Name {};
        /// Format of the input
        ElementType Type {};
        /// Byte offset of the start of the input
        u32 Offset {};
        /// Location for this input. Must match the location in the shader.
        u32 ShaderLocation {};
    };

    /// Describes how the vertex buffer is interpreted.
    /// For use in VertexState.
    struct VertexBufferLayout {
        /// The stride, in bytes, between elements of this buffer.
        u64 ArrayStride {};
        /// How often this vertex buffer is “stepped” forward.
        VertexStepMode StepMode {};
        /// The list of attributes which comprise a single vertex.
        std::vector<VertexAttribute> Attributes {};

        /// @param attributes The list of attributes which comprise a single vertex.
        static VertexBufferLayout Create(std::span<VertexAttribute> attributes)
        {
            VertexBufferLayout self;
            u32 offset = 0;
            for (auto& attr : attributes) {
                attr.Offset = offset;
                self.Attributes.push_back(attr);

                offset += element_type_count(attr.Type) * 4_u32;
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
        std::vector<VertexBufferLayout> AttributeLayouts {};
    };

    struct PrimitiveState {
        PrimitiveTopology Topology {};
        Maybe<IndexFormat> StripIndexFormat {};
        FrontFace FrontFace {};
        Face Cull {};

        static auto Default() -> PrimitiveState;
    };

    struct StencilFaceState {
        CompareFunction CompareFunc {};
        StencilOperation FailOp {};
        StencilOperation DepthFailOp {};
        StencilOperation PassOp {};
    };

    struct StencilState {
        StencilFaceState Front {};
        StencilFaceState Back {};
        u32 ReadMask {};
        u32 WriteMask {};
    };

    struct DepthBiasState {
        /// Constant depth biasing factor, in basic units of the depth format.
        s32 Constant {};

        /// Slope depth biasing factor.
        f32 SlopeScale {};

        /// Depth bias clamp value (absolute).
        f32 Clamp {};
    };

    struct DepthStencilState {
        TextureFormat Format {};
        bool DepthWriteEnabled {};
        CompareFunction DepthCompare {};
        StencilState Stencil {};
        DepthBiasState Bias {};

        static auto Default() -> DepthStencilState;
    };

    struct BlendComponent {
        BlendFactor SrcFactor {};
        BlendFactor DstFactor {};
        BlendOperation Operation {};
    };

    struct BlendState {
        BlendComponent Color {}, Alpha {};

        static auto Default() -> BlendState;
    };

    struct ColorTargetState {
        /// The TextureFormat of the image that this pipeline will render to.
        /// Must match the format of the corresponding color attachment in CommandEncoder::BeginRendering
        TextureFormat Format {};
        /// The blending that is used for this pipeline.
        Maybe<BlendState> Blend {};
        /// Mask which enables/disables writes to different color/alpha channel.
        ColorWriteFlags WriteMask {};
    };

    struct FragmentStage {
        std::vector<ColorTargetState> Targets {};
    };

    struct MultiSampleState {
        /// The number of samples calculated per pixel (for MSAA).
        /// For non-multi-sampled textures, this should be 1
        u32 Count { 1 };
        /// Bitmask that restricts the samples of a pixel modified by this pipeline.
        /// All samples can be enabled using the value !0
        u32 Mask {};
        /// When enabled, produces another sample mask per pixel based on the
        /// alpha output value, that is ANDed with the sample_mask and the
        /// primitive coverage to restrict the set of samples affected by a primitive.
        /// The implicit mask produced for alpha of zero is guaranteed to be zero,
        /// and for alpha of one is guaranteed to be all 1-s.
        bool AlphaToCoverageEnabled {};

        static auto Default() -> MultiSampleState;
    };

    struct RenderPipelineSpec {
        /// Debug label of the render pass. This will show up in graphics debuggers for easy identification.
        Maybe<std::string_view> Label {};
        Maybe<PipelineLayout> Layout {};
        VertexState Vertex {};
        PrimitiveState Primitive {};
        Maybe<DepthStencilState> DepthStencil {};
        MultiSampleState MultiSample {};
        Maybe<FragmentStage> Fragment {};
        /// Used if the shader module is a SpirV binary.
        Maybe<std::string_view> VertexEntryPointOverride {};
        /// Used if the shader module is a SpirV binary.
        Maybe<std::string_view> FragmentEntryPointOverride {};
        // MutliView
        // Cache
    };

    struct RenderPipeline : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        auto GetBindGroupLayout(u32 index) -> BindGroupLayout;
        virtual void Release() override;
    };

    struct RenderPassColorAttachment {
        TextureView View {};
        LoadOp LoadOp {};
        StoreOp StoreOp {};
        Color ClearColor {};
        f32 DepthClear {};
    };

    /// Describes the timestamp writes of a render pass.
    /// For use with RenderPassSpec.
    struct RenderPassTimestampWrites {
        /// The query set to write to.
        QuerySet Set {};
        /// The index of the query set at which a start timestamp of this pass is written, if any.
        Maybe<u32> BeginningOfPassWriteIndex {};
        /// The index of the query set at which an end timestamp of this pass is written, if any.
        Maybe<u32> EndOfPassWriteIndex {};
    };

    struct RenderPassSpec {
        /// Debug label of the render pass. This will show up in graphics debuggers for easy identification.
        Maybe<std::string_view> Label {};
        /// The color attachments of the render pass.
        std::span<RenderPassColorAttachment> ColorAttachments {};
        /// The depth and stencil attachment of the render pass, if any.
        Maybe<RenderPassColorAttachment> DepthStencilAttachment {};
        /// Defines which timestamp values will be written for this pass, and where to write them to.
        Maybe<RenderPassTimestampWrites> TimestampWrites {};
        // occlusion_query_set
    };

    struct RenderPassEncoder final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        /// Sets the viewport used during the rasterization stage to
        /// linearly map from normalized device coordinates to viewport coordinates.
        /// Subsequent draw calls will only draw within this region.
        /// If this method has not been called, the viewport defaults
        /// to the entire bounds of the render targets.
        void SetViewport(Vector2 const& origin, Vector2 const& size, f32 min_depth = 0.0f, f32 max_depth = 1.0f) const;

        void SetBindGroup(BindGroup const& group, u32 index) const;
        void SetVertexBuffer(u32 slot, BufferSlice const& slice) const;
        void SetIndexBuffer(BufferSlice const& slice) const;

        void SetPipeline(RenderPipeline const& pipeline) const;
        void Draw(Range<u32> vertices, Range<u32> instances) const;
        void DrawIndex(Range<u32> indices, Range<u32> instances) const;

        /// Start a pipeline statistics query on this render pass. It can be ended with end_pipeline_statistics_query().
        /// Pipeline statistics queries may not be nested.
        void BeginPipelineStatisticsQuery(QuerySet const& set, u32 index) const;
        /// End the pipeline statistics query on this render pass. It can be started with begin_pipeline_statistics_query().
        /// Pipeline statistics queries may not be nested.
        void EndPipelineStatisticsQuery() const;

        void InsertDebugMarker(String const& label) const;
        void PushDebugGroup(String const& name) const;
        void PopDebugGroup() const;

        void End() const;
        virtual void Release() override;
    };

    struct CommandBuffer {
        HandleT handle {};

        void Release() const;
    };

    struct CommandEncoder {
        HandleT handle {};

        [[nodiscard]]
        auto BeginRendering(RenderPassSpec const& spec) const -> RenderPassEncoder;
        auto Finish() -> CommandBuffer;

        void CopyBufferToBuffer(Buffer const& from, u64 from_offset, Buffer const& to, u64 to_offset, u64 size) const;
        void CopyTextureToTexture(
            Texture const& from,
            Texture const& to,
            Vector2 const& size,
            u32 from_mip_level = 0,
            u32 to_mip_level = 0,
            u32 from_array_index = 0,
            u32 to_array_index = 0) const;

        void ResolveQuerySet(
            QuerySet const& set,
            Range<u32> query_range,
            Buffer const& destination,
            u64 destination_offset) const;

        void PushDebugGroup(String const& name) const;
        void PopDebugGroup() const;

        void Release() const;
    };

    struct Limits {
        u32 MaxTextureDimension1D {};
        u32 MaxTextureDimension2D {};
        u32 MaxTextureDimension3D {};
        u32 MaxTextureArrayLayers {};
        u32 MaxBindGroups {};
        u32 MaxBindingsPerBindGroup {};
        u32 MaxDynamicUniformBuffersPerPipelineLayout {}; // 8,
        u32 MaxDynamicStorageBuffersPerPipelineLayout {}; // 4,
        u32 MaxSampledTexturesPerShaderStage {};          // 16,
        u32 MaxSamplersPerShaderStage {};                 // 16,
        u32 MaxStorageBuffersPerShaderStage {};           // 4, // *
        u32 MaxStorageTexturesPerShaderStage {};          // 4,
        u32 MaxUniformBuffersPerShaderStage {};           // 12,
        u32 MaxUniformBufferBindingSize {};               // 16 << 10, // * (16 KiB)
        u32 MaxStorageBufferBindingSize {};               // 128 << 20, // (128 MiB)
        u32 MaxVertexBuffers {};                          // 8,
        u32 MaxVertexAttributes {};                       // 16,
        u32 MaxVertexBufferArrayStride {};                // 2048,
        u32 MinSubgroupSize {};                           // 0,
        u32 MaxSubgroupSize {};                           // 0,
        u32 MaxPushConstantSize {};                       // 0,
        u32 MinUniformBufferOffsetAlignment {};           // 256,
        u32 MinStorageBufferOffsetAlignment {};           // 256,
        u32 MaxInterStageShaderComponents {};             // 60,
        u32 MaxColorAttachments {};                       // 8,
        u32 MaxColorAttachmentBytesPerSample {};          // 32,
        u32 MaxComputeWorkgroupStorageSize {};            // 16352, // *
        u32 MaxComputeInvocationsPerWorkgroup {};         // 256,
        u32 MaxComputeWorkgroupSizeX {};                  // 256,
        u32 MaxComputeWorkgroupSizeY {};                  // 256,
        u32 MaxComputeWorkgroupSizeZ {};                  // 64,
        u32 MaxComputeWorkgroupsPerDimension {};          // 65535,
        u64 MaxBufferSize {};
        u32 MaxNonSamplerBindings {};

        static Limits Default();
        static Limits DownlevelDefaults();
    };

    struct DeviceSpec final {
        Maybe<String> Label {};
        /// Specifies the features that are required by the device request. The request will fail if the adapter cannot provide these features.
        /// Exactly the specified set of features, and no more or less, will be allowed in validation of API calls on the resulting device.
        std::vector<Feature> RequiredFeatures {};
    };

    struct Device final : GPUHandle<void> {
        HandleT Queue {};

        Device() = default;
        explicit Device(HandleT handle);

        [[nodiscard]] auto CreateBuffer(BufferSpec const& spec) const -> Buffer;
        [[nodiscard]] auto CreateTexture(TextureSpec const& spec) const -> Texture;
        [[nodiscard]] auto CreateSampler(SamplerSpec const& spec) const -> Sampler;
        [[nodiscard]] auto CreateCommandEncoder(char const* label = "") const -> CommandEncoder;
        /// Create a bind group, duh.
        auto CreateBindGroup(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup;
        [[nodiscard]] auto CreateBindGroupLayout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout;
        [[nodiscard]] auto CreateQuerySet(QuerySetSpec const& spec) const -> QuerySet;

        [[nodiscard]] auto CreateShaderModule(ShaderModuleSpec const& spec) const -> ShaderModule;
#ifdef WGPU_DEV
        [[nodiscard]] auto CreateShaderModuleSpirV(SpirVShaderSpec const& spec) const -> ShaderModule;
#endif
        [[nodiscard]] auto CreatePipelineLayout(PipelineLayoutSpec const& spec) const -> PipelineLayout;
        [[nodiscard]] auto CreateRenderPipeline(ShaderModule const& vert_module, ShaderModule const& frag_module, RenderPipelineSpec const& spec) const -> RenderPipeline;

        void SubmitCommandBuffer(CommandBuffer cmd) const;

        /// Schedule a data write into buffer starting at offset.
        /// This method fails if data overruns the size of buffer starting at offset.
        /// This does not submit the transfer to the GPU immediately.
        /// Calls to write_buffer begin execution only on the next call to Queue::submit
        void WriteBuffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const;

        template<typename T>
        void WriteBuffer(Buffer const& buffer, u64 offset, std::span<T> const& data) const
        {
            WriteBuffer(buffer, offset, data.data(), data.size_bytes());
        }

        template<typename T>
        void WriteBuffer(Buffer const& buffer, u64 offset, std::vector<T> const& data) const
        {
            WriteBuffer(buffer, offset, data.data(), data.size() * sizeof(T));
        }

        void WriteTexture(
            Texture const& texture,
            void const* data,
            size_t data_size,
            Vector2 const& origin,
            Vector2 const& size,
            u32 bytes_per_pixel = 4,
            u32 mip_level = 0) const;

        virtual void Release() override;

    private:
        ErrorFn m_Function {};
    };

    struct AdapterOptions {
        DevicePower PowerPreference { DevicePower::HighPerformance };
    };

    struct Adapter {
        HandleT Handle {};

        auto RequestDevice(DeviceSpec const& spec) -> Device;

        bool HasFeature(Feature feature) const;

        void Release() const;
    };

    struct Surface {
        enum class Error {
            Timeout,
            Outdated,
            Lost,
            OutOfMemory,
        };

        struct Config {
            PresentMode Mode { PresentMode::Immediate };
            Vector2 Size {};
        };

        HandleT Handle {};
        TextureFormat Format {};

        void Release() const;

        void Configure(Device const& device, Adapter adapter, Config const& config);
        auto GetNextView() const -> Result<TextureView, Error>;
        void Present() const;
    };

    struct InstanceSpec {
        BackendRenderer Backend { BackendRenderer::Vulkan };
    };

    struct Instance {
        HandleT Handle {};

        static Instance Create(InstanceSpec const& spec = {});

        void Release() const;

        auto GetAdapter(Surface surface, AdapterOptions const& opt = {}) const -> Adapter;

        // TODO: Make this take a reference.
        auto GetSurface(Window const* window) const -> Surface;

        GlobalReport GenerateGlobalReport() const;
    };

    bool IsHDR(TextureFormat format);
}
