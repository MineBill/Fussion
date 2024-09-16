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
#include <memory_resource>

// TODO: Add tests for this
template<Fussion::ScalarType T>
struct Range {
    T start{};
    T stop{};

    T count() const { return stop - start; }

    class Iterator {
        T m_number{}, m_start{}, m_end{};

    public:
        explicit Iterator(T start, T end, T number = 0) : m_number(number), m_start(start), m_end(end) {}

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
        HandleT handle{};
        SpecT spec{};

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle): handle(handle) {}
        GPUHandle(HandleT handle, SpecT const& spec): handle(handle), spec(spec) {}

        template<typename T>
        auto as() const -> T
        {
            return CAST(T, handle);
        }

        virtual void release() = 0;
    };

    template<>
    struct GPUHandle<void> {
        HandleT handle{};

        virtual ~GPUHandle() = default;
        GPUHandle() = default;
        explicit GPUHandle(HandleT handle): handle(handle) {}

        template<typename T>
        auto as() const -> T
        {
            return CAST(T, handle);
        }

        virtual void release() = 0;
    };

    struct SamplerSpec {
        Maybe<std::string_view> label{};
        AddressMode address_mode_u{ AddressMode::Repeat };
        AddressMode address_mode_v{ AddressMode::Repeat };
        AddressMode address_mode_w{ AddressMode::Repeat };
        FilterMode mag_filter{ FilterMode::Linear };
        FilterMode min_filter{ FilterMode::Linear };
        FilterMode mip_map_filter{ FilterMode::Linear };
        f32 lod_min_clamp{ 0.0f };
        f32 lod_max_clamp{ 32.0f };
        Maybe<CompareFunction> compare{};
        u16 anisotropy_clamp{ 1_u16 };
    };

    struct Sampler : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void release() override;
    };

    struct BufferSpec {
        Maybe<std::string_view> label{};
        BufferUsageFlags usage{};
        u64 size{};
        bool mapped{ false };
    };

    struct BufferSlice;

    struct Buffer final : GPUHandle<BufferSpec> {
        using GPUHandle::GPUHandle;

        auto size() const -> u64;
        auto slice(u32 start, u32 size) const -> BufferSlice;
        auto slice() const -> BufferSlice;

        void unmap() const;

        virtual void release() override;
    };

    struct BufferSlice {
        using AsyncMapCallback = std::function<void()>;

        Buffer backing_buffer;
        u32 start{};
        u32 size{};

        BufferSlice(Buffer const& buffer, u32 start, u32 size);
        BufferSlice(Buffer const& buffer);

        /// Returns the slice as a mapped ranged.
        /// This assumes buffer was created with Mapped = true
        /// or that this call is a result from map of the buffer.
        auto mapped_range() -> void*;


        void map_async(AsyncMapCallback const& callback);
    };

    struct TextureSpec {
        Maybe<std::string_view> label{};
        TextureUsageFlags usage{};
        TextureDimension dimension{};
        Vector3 size{};
        TextureFormat format{};
        //u32 MipLevelCount{ 1 };
        u32 sample_count{ 1 };
        TextureAspect aspect{};

        bool generate_mip_maps{ false };
    };

    struct TextureViewSpec {
        Maybe<std::string_view> label{};
        TextureUsageFlags usage{};
        TextureViewDimension dimension{};
        TextureFormat format{};

        u32 base_mip_level{ 0 };
        u32 mip_level_count{ 1 };
        u32 base_array_layer{ 0 };
        u32 array_layer_count{ 1 };

        TextureAspect aspect{};
    };

    struct TextureView final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        operator HandleT() const
        {
            return handle;
        }

        virtual void release() override;
    };

    struct Texture final : GPUHandle<TextureSpec> {
        using GPUHandle::GPUHandle;

        u32 mip_level_count{};
        TextureView view{};

        void initialize_view(u32 array_count = 1);
        void generate_mipmaps(Device const& device);
        auto create_view(TextureViewSpec const& spec) const -> TextureView;

        virtual void release() override;
    };

    namespace BufferBindingType {
        struct Uniform {};

        struct Storage {
            bool read_only{};
        };

        using Type = std::variant<Uniform, Storage>;
    };

    namespace TextureSampleType {
        struct Float {
            bool filterable{ true };
        };

        struct Depth {};

        struct SInt {};

        struct UInt {};

        using Type = std::variant<Float, Depth, SInt, UInt>;
    };

    namespace BindingType {
        struct Buffer {
            /// Sub-type of the buffer binding.
            BufferBindingType::Type type{};
            /// Indicates that the binding has a dynamic offset.
            /// One offset must be passed to RenderPass::set_bind_group
            /// for each dynamic binding in increasing order of binding number.
            bool has_dynamic_offset{};
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
            Maybe<u64> min_binding_size{};
        };

        struct Sampler {
            SamplerBindingType type{};
        };

        struct Texture {
            TextureSampleType::Type sample_type{};
            TextureViewDimension view_dimension{};
            bool multi_sampled{ false };
        };

        struct StorageTexture {
            StorageAccess access{};
            TextureFormat format{};
            TextureViewDimension view_dimension{};
        };

        struct AccelerationStructure {};

        using Type = std::variant<Buffer, Sampler, Texture, StorageTexture, AccelerationStructure>;
    };

    struct BindGroupLayoutEntry {
        u32 binding{};
        ShaderStageFlags visibility{};
        BindingType::Type type{};
        Maybe<u32> count{};
    };

    struct BindGroupLayoutSpec {
        Maybe<std::string_view> label{};
        std::span<BindGroupLayoutEntry> entries{};
    };

    struct BindGroupLayout {
        HandleT handle{};
    };

    /// Binding for uniform or storage buffers.
    struct BufferBinding {
        /// The buffer to bind.
        Buffer buffer{};

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
        u64 offset{};
        /// Size of the binding in bytes, or None for using the rest of the buffer.
        u64 size{};
    };

    using BindingResource = std::variant<BufferBinding, TextureView, Sampler>;

    struct BindGroupEntry {
        u32 binding{};
        BindingResource resource;
    };

    struct BindGroupSpec {
        /// Label for the bind group.
        Maybe<std::string_view> label{};
        std::span<BindGroupEntry> entries{};
    };

    /// A BindGroup represents the set of resources bound to the bindings
    /// described by a BindGroupLayout. It can be created with Device::CreateBindGroup.
    /// A BindGroup can be bound to a particular RenderPass with RenderPassEncoder::SetBindGroup,
    /// or to a ComputePass with ComputePassEncoder::SetBindGroup.
    struct BindGroup : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void release() override;
    };

    struct PipelineLayoutSpec {
        Maybe<std::string_view> label{};
        std::span<BindGroupLayout> bind_group_layouts{};
    };

    struct PipelineLayout final : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        virtual void release() override;
    };

    struct WGSLShader {
        std::string_view source{};
    };

    struct SPIRVShader {
        std::span<u32> binary{};
    };

    using ShaderType = std::variant<WGSLShader, SPIRVShader>;

    struct ShaderModuleSpec {
        Maybe<std::string_view> label{};
        ShaderType type{};

        std::string_view vertex_entry_point{};
        std::string_view fragment_entry_point{};
    };

    struct ShaderModule final : GPUHandle<ShaderModuleSpec> {
        using GPUHandle::GPUHandle;

        virtual void release() override;
    };

    /// Vertex inputs (attributes) to shaders.
    struct VertexAttribute {
        /// Format of the input
        ElementType type{};
        /// Byte offset of the start of the input
        u32 offset{};
        /// Location for this input. Must match the location in the shader.
        u32 shader_location{};
    };

    /// Describes how the vertex buffer is interpreted.
    /// For use in VertexState.
    struct VertexBufferLayout {
        /// The stride, in bytes, between elements of this buffer.
        u64 array_stride{};
        /// How often this vertex buffer is “stepped” forward.
        VertexStepMode step_mode{};
        /// The list of attributes which comprise a single vertex.
        std::vector<VertexAttribute> attributes{};

        /// @param attributes The list of attributes which comprise a single vertex.
        static VertexBufferLayout create(std::span<VertexAttribute> attributes)
        {
            VertexBufferLayout self;
            u32 offset = 0;
            for (auto& attr : attributes) {
                attr.offset = offset;
                self.attributes.push_back(attr);

                offset += element_type_count(attr.type) * 4_u32;
            }
            self.array_stride = offset;
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
        std::vector<VertexBufferLayout> attribute_layouts{};
    };

    struct PrimitiveState {
        PrimitiveTopology topology{};
        Maybe<IndexFormat> strip_index_format{};
        FrontFace front_face{};
        Face cull{};

        static auto get_default() -> PrimitiveState;
    };

    struct StencilFaceState {
        CompareFunction compare{};
        StencilOperation fail_op{};
        StencilOperation depth_fail_op{};
        StencilOperation pass_op{};
    };

    struct StencilState {
        StencilFaceState front{};
        StencilFaceState back{};
        u32 read_mask{};
        u32 write_mask{};
    };

    struct DepthBiasState {
        /// Constant depth biasing factor, in basic units of the depth format.
        s32 constant{};

        /// Slope depth biasing factor.
        f32 slope_scale{};

        /// Depth bias clamp value (absolute).
        f32 clamp{};
    };

    struct DepthStencilState {
        TextureFormat format{};
        bool depth_write_enabled{};
        CompareFunction depth_compare{};
        StencilState stencil{};
        DepthBiasState bias{};

        static auto get_default() -> DepthStencilState;
    };

    struct BlendComponent {
        BlendFactor src_factor{};
        BlendFactor dst_factor{};
        BlendOperation operation{};
    };

    struct BlendState {
        BlendComponent color{}, alpha{};

        static auto get_default() -> BlendState;
    };

    struct ColorTargetState {
        /// The TextureFormat of the image that this pipeline will render to.
        /// Must match the format of the corresponding color attachment in CommandEncoder::BeginRendering
        TextureFormat format{};
        /// The blending that is used for this pipeline.
        Maybe<BlendState> blend{};
        /// Mask which enables/disables writes to different color/alpha channel.
        ColorWriteFlags write_mask{};
    };

    struct FragmentStage {
        std::vector<ColorTargetState> targets{};
    };

    struct MultiSampleState {
        /// The number of samples calculated per pixel (for MSAA).
        /// For non-multi-sampled textures, this should be 1
        u32 count{};
        /// Bitmask that restricts the samples of a pixel modified by this pipeline.
        /// All samples can be enabled using the value !0
        u32 mask{};
        /// When enabled, produces another sample mask per pixel based on the
        /// alpha output value, that is ANDed with the sample_mask and the
        /// primitive coverage to restrict the set of samples affected by a primitive.
        /// The implicit mask produced for alpha of zero is guaranteed to be zero,
        /// and for alpha of one is guaranteed to be all 1-s.
        bool alpha_to_coverage_enabled{};

        static auto get_default() -> MultiSampleState;
    };

    struct RenderPipelineSpec {
        Maybe<std::string_view> label{};
        Maybe<PipelineLayout> layout{};
        VertexState vertex{};
        PrimitiveState primitive{};
        Maybe<DepthStencilState> depth_stencil{};
        MultiSampleState multi_sample{};
        Maybe<FragmentStage> fragment{};
        // MutliView
        // Cache
    };

    struct RenderPipeline : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        auto bind_group_layout(u32 index) -> BindGroupLayout;
        virtual void release() override;
    };

    struct RenderPassColorAttachment {
        TextureView view{};
        LoadOp load_op{};
        StoreOp store_op{};
        Color clear_color{};
        f32 depth_clear{};
    };

    struct RenderPassSpec {
        Maybe<std::string_view> label{};
        std::span<RenderPassColorAttachment> color_attachments{};
        Maybe<RenderPassColorAttachment> depth_stencil_attachment{};
    };

    struct RenderPassEncoder : GPUHandle<void> {
        using GPUHandle::GPUHandle;

        /// Sets the viewport used during the rasterization stage to
        /// linearly map from normalized device coordinates to viewport coordinates.
        /// Subsequent draw calls will only draw within this region.
        /// If this method has not been called, the viewport defaults
        /// to the entire bounds of the render targets.
        void set_viewport(Vector2 const& origin, Vector2 const& size, f32 min_depth = 0.0f, f32 max_depth = 1.0f) const;

        void set_bind_group(BindGroup group, u32 index) const;
        void set_vertex_buffer(u32 slot, BufferSlice const& slice) const;
        void set_index_buffer(BufferSlice const& slice) const;
        // void SetStorageBuffer(u32 slot, BufferSlice const& slice) const;

        void set_pipeline(RenderPipeline const& pipeline) const;
        void draw(Range<u32> vertices, Range<u32> instances) const;
        void draw_index(Range<u32> indices, Range<u32> instances) const;

        void end() const;
        virtual void release() override;
    };

    struct CommandBuffer {
        HandleT handle{};

        void release() const;
    };

    struct CommandEncoder {
        HandleT handle{};

        auto begin_rendering(RenderPassSpec const& spec) const -> RenderPassEncoder;
        auto finish() -> CommandBuffer;

        void copy_buffer_to_buffer(Buffer const& from, u64 from_offset, Buffer const& to, u64 to_offset, u64 size) const;
        void copy_texture_to_texture(Texture const& from, Texture const& to, Vector2 const& size, u32 from_mip_level = 0, u32 to_mip_level = 0) const;

        void release() const;
    };

    struct Device final : GPUHandle<void> {
        HandleT queue{};

        Device() = default;
        explicit Device(HandleT handle);

        // void SetErrorCallback(ErrorFn const& function);

        auto create_buffer(BufferSpec const& spec) const -> Buffer;
        auto create_texture(TextureSpec const& spec) const -> Texture;
        auto create_sampler(SamplerSpec const& spec) const -> Sampler;
        auto create_command_encoder(const char* label = "") const -> CommandEncoder;
        auto create_bind_group(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup;
        auto create_bind_group_layout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout;

        auto create_shader_module(ShaderModuleSpec const& spec) const -> ShaderModule;
        auto create_pipeline_layout(PipelineLayoutSpec const& spec) const -> PipelineLayout;
        auto create_render_pipeline(ShaderModule const& module, RenderPipelineSpec const& spec) const -> RenderPipeline;

        void submit_command_buffer(CommandBuffer cmd) const;

        /// Schedule a data write into buffer starting at offset.
        /// This method fails if data overruns the size of buffer starting at offset.
        /// This does not submit the transfer to the GPU immediately.
        /// Calls to write_buffer begin execution only on the next call to Queue::submit
        void write_buffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const;

        template<typename T>
        void write_buffer(Buffer const& buffer, u64 offset, std::span<T> data) const
        {
            write_buffer(buffer, offset, data.data(), data.size_bytes());
        }

        void write_texture(
            Texture const& texture,
            void const* data,
            size_t data_size,
            Vector2 const& origin,
            Vector2 const& size,
            u32 bytes_per_pixel = 4,
            u32 mip_level = 0) const;

        virtual void release() override;

    private:
        ErrorFn m_function{};
    };

    struct AdapterOptions {
        DevicePower power_preference{ DevicePower::HighPerformance };
    };

    struct Adapter {
        HandleT handle{};

        auto device() -> Device;

        void release() const;
    };

    struct Surface {
        enum class Error {
            Timeout,
            Outdated,
            Lost,
            OutOfMemory,
        };

        struct Config {
            PresentMode present_mode{ PresentMode::Immediate };
            Vector2 size{};
        };

        HandleT handle{};
        TextureFormat format{};

        void release() const;

        void configure(Device const& device, Adapter adapter, Config const& config);
        auto get_next_view() const -> Result<TextureView, Error>;
        void present() const;
    };

    struct InstanceSpec {
        BackendRenderer backend{ BackendRenderer::Vulkan };
    };

    struct Instance {
        HandleT handle{};

        static Instance create(InstanceSpec const& spec = {});

        void release() const;

        auto adapter(Surface surface, AdapterOptions const& opt = {}) const -> Adapter;

        // TODO: Make this take a reference.
        auto surface(Window const* window) const -> Surface;
    };


}
