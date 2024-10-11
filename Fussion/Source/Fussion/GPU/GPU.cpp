#include "GPU.h"

#include "FussionPCH.h"
#include "EnumConversions.h"
#include "Utils.h"
#include "glfw3webgpu.h"

#include <GLFW/glfw3.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

constexpr auto MipMapGeneratorSource = R"wgsl(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) vtx: u32) -> VertexOutput {
    var out: VertexOutput;
    var plane: array<vec3f, 6> = array(
        vec3f(-1, -1, 0), vec3f(1, -1, 0), vec3f(-1, 1, 0),
        vec3f(-1, 1, 0), vec3f(1, -1, 0), vec3f(1, 1, 0)
    );
    var uvs: array<vec2<f32>, 6> = array(
        vec2f(0, 1), vec2f(1, 1), vec2f(0, 0),
        vec2f(0, 0), vec2f(1, 1), vec2f(1, 0),
    );
    let p = plane[vtx].xyz;

    out.position = vec4<f32>(p, 1.0);
    out.uv = uvs[vtx];
    // out.uv = vec2f(f32((vtx << 1) & 2), f32(vtx & 2));
    // out.position = vec4f(out.uv * vec2f(2.0f, -2.0f) + vec2f( -1.0f, 1.0f), 0.0f, 1.0f);
    return out;
}

@group(0) @binding(0) var tex: texture_2d<f32>;
@group(0) @binding(1) var sam: sampler;

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return textureSample(tex, sam, in.uv);
})wgsl";

namespace Fussion::GPU {
    template<class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    struct MipMapPipeline {
        ShaderModule shader {};
        RenderPipeline pipeline {};
        BindGroupLayout layout {};
        BindGroup bind_group {};
        Sampler sampler {};
        Texture render_texture {};
        Texture target_texture {};
        u32 mip_levels {};
        std::vector<TextureView> views {};

        void Initalize(Device const& device, Texture& texture)
        {
            target_texture = texture;
            {
                Vector2 size = { texture.Spec.Size.x, texture.Spec.Size.y };
                render_texture = device.CreateTexture({
                    .Usage = TextureUsage::CopySrc | TextureUsage::RenderAttachment | TextureUsage::CopyDst,
                    .Dimension = TextureDimension::D2,
                    .Size = { texture.Spec.Size.x, texture.Spec.Size.y, 1 },
                    .Format = texture.Spec.Format,
                    .SampleCount = 1,
                    .Aspect = TextureAspect::All,
                    .GenerateMipMaps = true,
                });
                render_texture.InitializeView();

                auto encoder = device.CreateCommandEncoder();
                encoder.CopyTextureToTexture(texture, render_texture, size);
                auto cmd = encoder.Finish();
                device.SubmitCommandBuffer(cmd);
            }
            mip_levels = texture.MipLevelCount;
            ShaderModuleSpec shader_spec {
                .Label = "MipMap Generator"sv,
                .Type = GPU::WGSLShader {
                    .Source = MipMapGeneratorSource,
                },
                .VertexEntryPoint = "vs_main",
                .FragmentEntryPoint = "fs_main",
            };
            shader = device.CreateShaderModule(shader_spec);

            RenderPipelineSpec spec {
                .Label = "fuck"sv,
                .Layout = None(),
                .Primitive = {
                    .Topology = PrimitiveTopology::TriangleList,
                    .StripIndexFormat = None(),
                    .FrontFace = FrontFace::Ccw,
                    .Cull = Face::None,
                },
                .DepthStencil = None(),
                .MultiSample = MultiSampleState::Default(),
                .Fragment = FragmentStage {
                    .Targets = { ColorTargetState {
                        .Format = texture.Spec.Format,
                        .Blend = BlendState::Default(),
                        .WriteMask = ColorWrite::All,
                    } },
                },
            };
            spec.Primitive.Topology = PrimitiveTopology::TriangleStrip;

            pipeline = device.CreateRenderPipeline(shader, shader, spec);

            layout = pipeline.GetBindGroupLayout(0);

            SamplerSpec sampler_spec {
                .LodMinClamp = 0.f,
                .LodMaxClamp = 0.f,
                .AnisotropyClamp = 2,
            };
            sampler = device.CreateSampler(sampler_spec);

            std::array entries = {
                BindGroupEntry {
                    .Binding = 0,
                    .Resource = texture.View,
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = sampler },
            };
            BindGroupSpec bg_spec {
                .Label = "fuck2"sv,
                .Entries = entries,
            };

            bind_group = device.CreateBindGroup(layout, bg_spec);

            // NOTE: When this is called from another thread logging
            //       stuff here causes problems like segfaults and
            //       other race condition fun stuff.

            for (u32 i = 1; i < mip_levels; ++i) {
                views.emplace_back(render_texture.CreateView({ .Label = "View"sv,
                    .Usage = render_texture.Spec.Usage,
                    .Dimension = TextureViewDimension::D2,
                    .Format = render_texture.Spec.Format,
                    .BaseMipLevel = i,
                    .MipLevelCount = 1,
                    .BaseArrayLayer = 0,
                    .ArrayLayerCount = 1,
                    .Aspect = render_texture.Spec.Aspect }));
            }
        }

        void Process(Device const& device)
        {
            auto encoder = device.CreateCommandEncoder("MipMap Generation");
            u32 i = 1;
            Vector2 size = render_texture.Spec.Size;
            for (auto& view : views) {
                if (size.x > 1)
                    size.x = CAST(f32, CAST(u32, size.x) / 2);
                if (size.y > 1)
                    size.y = CAST(f32, CAST(u32, size.y) / 2);

                std::array colors = {
                    RenderPassColorAttachment {
                        .View = view,
                        .LoadOp = LoadOp::Clear,
                        .StoreOp = StoreOp::Store,
                        .ClearColor = Color::Magenta,
                    },
                };
                RenderPassSpec spec {
                    .ColorAttachments = colors
                };
                auto rp = encoder.BeginRendering(spec);

                rp.SetPipeline(pipeline);
                rp.SetBindGroup(bind_group, 0);
                rp.Draw({ 0, 6 }, { 0, 1 });
                rp.End();
                rp.Release();

                encoder.CopyTextureToTexture(render_texture, target_texture, size, i, i);
                ++i;
            }

            auto cmd = encoder.Finish();
            device.SubmitCommandBuffer(cmd);

            encoder.Release();
            for (auto& view : views) {
                view.Release();
            }
            pipeline.Release();
            sampler.Release();
            bind_group.Release();
            shader.Release();
        }
    };

    // Utility function to retrieve the adapter without callbacks.
    WGPUAdapter request_adapter_sync(WGPUInstance instance, WGPURequestAdapterOptions const* options)
    {
        struct UserData {
            WGPUAdapter adapter;
            bool request_ended;
        } user_data;
        wgpuInstanceRequestAdapter(
            instance, options,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message,
                void* p_user_data) {
                auto user_data = static_cast<UserData*>(p_user_data);

                if (status == WGPURequestAdapterStatus_Success) {
                    user_data->adapter = adapter;
                } else {
                    LOG_ERRORF("Could not get adapter: {}", message);
                }
                user_data->request_ended = true;
            },
            &user_data);

        return user_data.adapter;
    }

    // Utility function to retrieve the device without callbacks.
    WGPUDevice request_device_sync(WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor)
    {
        struct UserData {
            WGPUDevice device;
            bool request_ended;
        } user_data;

        wgpuAdapterRequestDevice(
            adapter, descriptor,
            [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message,
                void* p_user_data) {
                auto user_data = static_cast<UserData*>(p_user_data);

                if (status == WGPURequestDeviceStatus_Success) {
                    user_data->device = device;
                } else {
                    LOG_ERRORF("Could not get device: {}", message);
                }
                user_data->request_ended = true;
            },
            &user_data);

        return user_data.device;
    }

    void QuerySet::Release()
    {
        wgpuQuerySetRelease(As<WGPUQuerySet>());
    }

    void Sampler::Release()
    {
        if (Handle != nullptr) {
            wgpuSamplerRelease(As<WGPUSampler>());
        }
    }

    Buffer::Buffer(HandleT handle, BufferSpec const& spec)
        : GPUHandle(handle, spec)
    {
        if (spec.Mapped)
            m_CurrentMapState = MapState::Mapped;
    }

    auto Buffer::Size() const -> u64
    {
        return wgpuBufferGetSize(CAST(WGPUBuffer, Handle));
    }

    auto Buffer::Slice(u32 start, u32 size) -> BufferSlice
    {
        return BufferSlice(*this, start, size);
    }

    auto Buffer::Slice() -> BufferSlice
    {
        return BufferSlice(*this, 0, CAST(u32, Size()));
    }

    auto Buffer::GetMapState() const -> MapState
    {
        return m_CurrentMapState;
    }

    void Buffer::Unmap()
    {
        wgpuBufferUnmap(As<WGPUBuffer>());
        m_CurrentMapState = MapState::Unmapped;
    }

    void Buffer::Release()
    {
        wgpuBufferRelease(CAST(WGPUBuffer, Handle));
    }

    void Buffer::ForceMapState(MapState state)
    {
        m_CurrentMapState = state;
    }

    BufferSlice::BufferSlice(Buffer& buffer, u32 start, u32 size)
        : BackingBuffer(&buffer)
        , Start(start)
        , Size(size)
    {
    }

    BufferSlice::BufferSlice(Buffer& buffer)
        : BackingBuffer(&buffer)
        , Size(CAST(u32, buffer.Size()))
    {
    }

    auto BufferSlice::MappedRange() -> void*
    {
        return wgpuBufferGetMappedRange(BackingBuffer->As<WGPUBuffer>(), Start, Size);
    }

    void BufferSlice::MapAsync(MapModeFlags map_mode, AsyncMapCallback const& callback) const
    {
        struct UserData {
            Buffer* buffer;
            AsyncMapCallback callback;
        };
        BackingBuffer->m_CurrentMapState = MapState::Pending;

        wgpuBufferMapAsync(BackingBuffer->As<WGPUBuffer>(), ToWGPU(map_mode), Start, Size, [](WGPUBufferMapAsyncStatus status, void* userdata) {
            auto user_data = CAST(UserData*, userdata);
            if (status == WGPUBufferMapAsyncStatus_Success) {
                user_data->buffer->m_CurrentMapState = MapState::Mapped;
                user_data->callback();
            } else {
                LOG_ERRORF("Could not map buffer: {}", magic_enum::enum_name(status));
            }

            delete user_data; }, new UserData(BackingBuffer, callback));
    }

    void TextureView::Release()
    {
        wgpuTextureViewRelease(CAST(WGPUTextureView, Handle));
    }

    void Texture::InitializeView(u32 array_count)
    {
        View = CreateView({
            .Label = "View"sv,
            .Usage = Spec.Usage,
            .Dimension = array_count == 1 ? TextureViewDimension::D2 : TextureViewDimension::D2_Array, // TODO: Make configurable
            .Format = Spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = MipLevelCount,
            .BaseArrayLayer = 0,            // TODO: Make configurable
            .ArrayLayerCount = array_count, // TODO: Make configurable
            .Aspect = Spec.Aspect           // TODO: Make configurable
        });
    }

    void Texture::GenerateMipmaps(Device const& device)
    {
        if (Spec.GenerateMipMaps && MipLevelCount > 1) {
            MipMapPipeline mmp;
            mmp.Initalize(device, *this);

            // Utils::RenderDoc::StartCapture();
            mmp.Process(device);
            // Utils::RenderDoc::EndCapture();
        }
    }

    TextureView Texture::CreateView(TextureViewSpec const& spec) const
    {
        WGPUTextureViewDescriptor texture_view_descriptor {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("View"sv).data(),
            .format = ToWGPU(spec.Format),
            .dimension = ToWGPU(spec.Dimension),
            .baseMipLevel = spec.BaseMipLevel,
            .mipLevelCount = spec.MipLevelCount,
            .baseArrayLayer = spec.BaseArrayLayer,
            .arrayLayerCount = spec.ArrayLayerCount,
            .aspect = ToWGPU(spec.Aspect),
        };

        auto view = wgpuTextureCreateView(CAST(WGPUTexture, Handle), &texture_view_descriptor);
        return TextureView { view };
    }

    void Texture::Release()
    {
        if (!Handle)
            return;
        wgpuTextureRelease(CAST(WGPUTexture, Handle));
        if (View != nullptr) {
            View.Release();
        }
    }

    void BindGroup::Release()
    {
        if (!Handle)
            return;
        wgpuBindGroupRelease(As<WGPUBindGroup>());
    }

    void PipelineLayout::Release()
    {
        wgpuPipelineLayoutRelease(As<WGPUPipelineLayout>());
    }

    void ShaderModule::Release()
    {
        wgpuShaderModuleRelease(As<WGPUShaderModule>());
    }

    auto PrimitiveState::Default() -> PrimitiveState
    {
        return PrimitiveState {
            .Topology = PrimitiveTopology::TriangleList,
            .StripIndexFormat = None(),
            .FrontFace = FrontFace::Ccw,
            .Cull = Face::None,
        };
    }

    auto DepthStencilState::Default() -> DepthStencilState
    {
        return {
            .Format = TextureFormat::Depth24Plus,
            .DepthWriteEnabled = true,
            .DepthCompare = CompareFunction::Less,
            .Stencil = {
                .Front = {
                    .CompareFunc = CompareFunction::Always,
                    .FailOp = StencilOperation::Keep,
                    .DepthFailOp = StencilOperation::Keep,
                    .PassOp = StencilOperation::Keep },
                .Back = { .CompareFunc = CompareFunction::Always, .FailOp = StencilOperation::Keep, .DepthFailOp = StencilOperation::Keep, .PassOp = StencilOperation::Keep },
                .ReadMask = 0xFFFFFFFF,
                .WriteMask = 0xFFFFFFFF,
            },
            .Bias = {
                .Constant = 0,
                .SlopeScale = 0,
                .Clamp = 0,
            }
        };
    }

    auto BlendState::Default() -> BlendState
    {
        return {
            .Color = {
                .SrcFactor = BlendFactor::SrcAlpha,
                .DstFactor = BlendFactor::OneMinusSrcAlpha,
                .Operation = BlendOperation::Add },
            .Alpha = { .SrcFactor = BlendFactor::Zero, .DstFactor = BlendFactor::One, .Operation = BlendOperation::Add }
        };
    }

    auto MultiSampleState::Default() -> MultiSampleState
    {
        return {
            .Count = 1,
            .Mask = ~0u,
            .AlphaToCoverageEnabled = false,
        };
    }

    auto RenderPipeline::GetBindGroupLayout(u32 index) -> BindGroupLayout
    {
        auto layout = wgpuRenderPipelineGetBindGroupLayout(As<WGPURenderPipeline>(), index);
        return BindGroupLayout { layout };
    }

    void RenderPipeline::Release()
    {
        wgpuRenderPipelineRelease(As<WGPURenderPipeline>());
    }

    void RenderPassEncoder::SetViewport(Vector2 const& origin, Vector2 const& size, f32 min_depth, f32 max_depth) const
    {
        wgpuRenderPassEncoderSetViewport(As<WGPURenderPassEncoder>(), origin.x, origin.y, size.x, size.y, min_depth, max_depth);
    }

    void RenderPassEncoder::SetBindGroup(BindGroup const& group, u32 index) const
    {
        wgpuRenderPassEncoderSetBindGroup(
            As<WGPURenderPassEncoder>(),
            index,
            CAST(WGPUBindGroup, group.Handle),
            0,        // TODO: Make configurable
            nullptr); // TODO: Make configurable
    }

    void RenderPassEncoder::SetVertexBuffer(u32 slot, BufferSlice const& slice) const
    {
        wgpuRenderPassEncoderSetVertexBuffer(As<WGPURenderPassEncoder>(), slot, slice.BackingBuffer->As<WGPUBuffer>(), slice.Start, slice.Size);
    }

    void RenderPassEncoder::SetIndexBuffer(BufferSlice const& slice) const
    {
        wgpuRenderPassEncoderSetIndexBuffer(As<WGPURenderPassEncoder>(), slice.BackingBuffer->As<WGPUBuffer>(), WGPUIndexFormat_Uint32, slice.Start, slice.Size);
    }

    void RenderPassEncoder::SetPipeline(RenderPipeline const& pipeline) const
    {
        wgpuRenderPassEncoderSetPipeline(As<WGPURenderPassEncoder>(), pipeline.As<WGPURenderPipeline>());
    }

    void RenderPassEncoder::Draw(Range<u32> vertices, Range<u32> instances) const
    {
        wgpuRenderPassEncoderDraw(As<WGPURenderPassEncoder>(), vertices.count(), instances.count(), vertices.start, instances.start);
    }

    void RenderPassEncoder::DrawIndex(Range<u32> indices, Range<u32> instances) const
    {
        wgpuRenderPassEncoderDrawIndexed(
            As<WGPURenderPassEncoder>(),
            indices.count(),
            instances.stop,
            indices.start,
            0,
            instances.start);
    }

    void RenderPassEncoder::BeginPipelineStatisticsQuery(QuerySet const& set, u32 index) const
    {
        wgpuRenderPassEncoderBeginPipelineStatisticsQuery(As<WGPURenderPassEncoder>(), set.As<WGPUQuerySet>(), index);
    }

    void RenderPassEncoder::EndPipelineStatisticsQuery() const
    {
        wgpuRenderPassEncoderEndPipelineStatisticsQuery(As<WGPURenderPassEncoder>());
    }

    void RenderPassEncoder::InsertDebugMarker(String const& label) const
    {
        wgpuRenderPassEncoderInsertDebugMarker(As<WGPURenderPassEncoder>(), label.data.ptr);
    }

    void RenderPassEncoder::PushDebugGroup(String const& name) const
    {
        wgpuRenderPassEncoderPushDebugGroup(As<WGPURenderPassEncoder>(), name.data.ptr);
    }

    void RenderPassEncoder::PopDebugGroup() const
    {
        wgpuRenderPassEncoderPopDebugGroup(As<WGPURenderPassEncoder>());
    }

    void RenderPassEncoder::End() const
    {
        wgpuRenderPassEncoderEnd(As<WGPURenderPassEncoder>());
    }

    void RenderPassEncoder::Release()
    {
        wgpuRenderPassEncoderRelease(As<WGPURenderPassEncoder>());
    }

    auto CommandEncoder::BeginRendering(RenderPassSpec const& spec) const -> RenderPassEncoder
    {
        std::array<WGPURenderPassColorAttachment, 10> stack_attachments {};

        int i = 0;
        for (auto const& attachment : spec.ColorAttachments) {
            stack_attachments[i++] = {
                .nextInChain = nullptr,
                .view = CAST(WGPUTextureView, attachment.View.Handle),
                .resolveTarget = nullptr,
                .loadOp = ToWGPU(attachment.LoadOp),
                .storeOp = ToWGPU(attachment.StoreOp),
                .clearValue = WGPUColor {
                    attachment.ClearColor.r,
                    attachment.ClearColor.g,
                    attachment.ClearColor.b,
                    attachment.ClearColor.a,
                },
            };
        }

        WGPURenderPassDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Render Pass"sv).data(),
            .colorAttachmentCount = spec.ColorAttachments.size(),
            .colorAttachments = stack_attachments.data(),
            .depthStencilAttachment = nullptr,
            .occlusionQuerySet = nullptr,
            .timestampWrites = nullptr,
        };

        WGPURenderPassTimestampWrites timestamp_writes {};
        if (spec.TimestampWrites) {
            timestamp_writes.querySet = spec.TimestampWrites->Set.As<WGPUQuerySet>();
            if (spec.TimestampWrites->BeginningOfPassWriteIndex) {
                timestamp_writes.beginningOfPassWriteIndex = spec.TimestampWrites->BeginningOfPassWriteIndex.Unwrap();
            }
            if (spec.TimestampWrites->EndOfPassWriteIndex) {
                timestamp_writes.endOfPassWriteIndex = spec.TimestampWrites->EndOfPassWriteIndex.Unwrap();
            }
            desc.timestampWrites = &timestamp_writes;
        }

        if (auto depth = spec.DepthStencilAttachment) {
            WGPURenderPassDepthStencilAttachment d {
                .view = CAST(WGPUTextureView, depth->View.Handle),
                .depthLoadOp = ToWGPU(depth->LoadOp),
                .depthStoreOp = ToWGPU(depth->StoreOp),
                .depthClearValue = depth->DepthClear,
                // .depthReadOnly = ,
                .stencilLoadOp = ToWGPU(LoadOp::Undefined),
                .stencilStoreOp = ToWGPU(StoreOp::Undefined),
                .stencilClearValue = 0,
                // .stencilReadOnly =
            };
            desc.depthStencilAttachment = &d;
            auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, handle), &desc);
            return RenderPassEncoder { rp };
        }
        auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, handle), &desc);
        return RenderPassEncoder { rp };
    }

    auto CommandEncoder::Finish() -> CommandBuffer
    {
        WGPUCommandBufferDescriptor cmd_buffer_descriptor {
            .nextInChain = nullptr,
            .label = "Pepe Command Buffer"
        };
        auto cmd = wgpuCommandEncoderFinish(CAST(WGPUCommandEncoder, handle), &cmd_buffer_descriptor);
        return CommandBuffer { cmd };
    }

    void CommandEncoder::CopyBufferToBuffer(Buffer const& from, u64 from_offset, Buffer const& to, u64 to_offset, u64 size) const
    {
        wgpuCommandEncoderCopyBufferToBuffer(CAST(WGPUCommandEncoder, handle), from.As<WGPUBuffer>(), from_offset, to.As<WGPUBuffer>(), to_offset, size);
    }

    void CommandEncoder::CopyTextureToTexture(
        Texture const& from,
        Texture const& to,
        Vector2 const& size,
        u32 from_mip_level,
        u32 to_mip_level,
        u32 from_array_index,
        u32 to_array_index) const
    {
        WGPUImageCopyTexture source {
            .nextInChain = nullptr,
            .texture = from.As<WGPUTexture>(),
            .mipLevel = from_mip_level,
            .origin = { 0, 0, from_array_index },
            .aspect = WGPUTextureAspect_All,
        };
        WGPUImageCopyTexture dest {
            .nextInChain = nullptr,
            .texture = to.As<WGPUTexture>(),
            .mipLevel = to_mip_level,
            .origin = { 0, 0, to_array_index },
            .aspect = WGPUTextureAspect_All,
        };
        WGPUExtent3D copy_size {
            .width = CAST(u32, size.x),
            .height = CAST(u32, size.y),
            .depthOrArrayLayers = 1,
        };

        wgpuCommandEncoderCopyTextureToTexture(CAST(WGPUCommandEncoder, handle), &source, &dest, &copy_size);
    }

    void CommandEncoder::ResolveQuerySet(
        QuerySet const& set,
        Range<u32> query_range,
        Buffer const& destination,
        u64 destination_offset) const
    {
        wgpuCommandEncoderResolveQuerySet(
            CAST(WGPUCommandEncoder, handle),
            set.As<WGPUQuerySet>(),
            query_range.start, query_range.count(),
            destination.As<WGPUBuffer>(),
            destination_offset);
    }

    void CommandEncoder::PushDebugGroup(String const& name) const
    {
        wgpuCommandEncoderPushDebugGroup(CAST(WGPUCommandEncoder, handle), name.data.ptr);
    }

    void CommandEncoder::PopDebugGroup() const
    {
        wgpuCommandEncoderPopDebugGroup(CAST(WGPUCommandEncoder, handle));
    }

    void CommandEncoder::Release() const
    {
        wgpuCommandEncoderRelease(CAST(WGPUCommandEncoder, handle));
    }

    Limits Limits::Default()
    {
        return {};
    }

    Limits Limits::DownlevelDefaults()
    {
        return {};
    }

    Device::Device(HandleT handle)
        : GPUHandle(handle)
    {
        Queue = wgpuDeviceGetQueue(CAST(WGPUDevice, handle));
    }

    auto Device::CreateBuffer(BufferSpec const& spec) const -> Buffer
    {
        WGPUBufferDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Buffer"sv).data(),
            .usage = ToWGPU(spec.Usage),
            .size = spec.Size,
            .mappedAtCreation = spec.Mapped
        };

        auto buffer = wgpuDeviceCreateBuffer(CAST(WGPUDevice, Handle), &desc);
        return Buffer { buffer, spec };
    }

    auto Device::CreateTexture(TextureSpec const& spec) const -> Texture
    {
        WGPUTextureDescriptor texture_descriptor {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Texture"sv).data(),
            .usage = ToWGPU(spec.Usage),
            .dimension = ToWGPU(spec.Dimension),
            .size = {
                .width = CAST(u32, spec.Size.x),
                .height = CAST(u32, spec.Size.y),
                .depthOrArrayLayers = CAST(u32, spec.Size.z),
            },
            .format = ToWGPU(spec.Format),
            .mipLevelCount = 1,
            .sampleCount = spec.SampleCount,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
        };

        if (spec.GenerateMipMaps) {
            texture_descriptor.mipLevelCount = CAST(u32, Math::FloorLog2(Math::Max(CAST(s32, spec.Size.x), CAST(s32, spec.Size.y)))) + 1;
        }

        auto texture_handle = wgpuDeviceCreateTexture(CAST(WGPUDevice, Handle), &texture_descriptor);

        Texture texture { texture_handle, spec };
        texture.MipLevelCount = texture_descriptor.mipLevelCount;
        if (spec.InitializeView) {
            texture.InitializeView();
        }
        return texture;
    }

    auto Device::CreateSampler(SamplerSpec const& spec) const -> Sampler
    {
        WGPUSamplerDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.label.ValueOr("Sampler"sv).data(),
            .addressModeU = ToWGPU(spec.AddressModeU),
            .addressModeV = ToWGPU(spec.AddressModeV),
            .addressModeW = ToWGPU(spec.AddressModeW),
            .magFilter = ToWGPU(spec.MagFilter),
            .minFilter = ToWGPU(spec.MinFilter),
            .mipmapFilter = CAST(WGPUMipmapFilterMode, ToWGPU(spec.MipMapFilter)),
            .lodMinClamp = spec.LodMinClamp,
            .lodMaxClamp = spec.LodMaxClamp,
            .compare = ToWGPU(spec.CompareFunc.ValueOr(CompareFunction::Undefined)),
            .maxAnisotropy = spec.AnisotropyClamp,
        };

        auto sampler = wgpuDeviceCreateSampler(As<WGPUDevice>(), &desc);
        return Sampler { sampler };
    }

    auto Device::CreateCommandEncoder(char const* label) const -> CommandEncoder
    {
        WGPUCommandEncoderDescriptor encoder_desc = {
            .nextInChain = nullptr,
            .label = label,
        };
        auto encoder = wgpuDeviceCreateCommandEncoder(CAST(WGPUDevice, Handle), &encoder_desc);
        return CommandEncoder { encoder };
    }

    auto Device::CreateBindGroup(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup
    {
        std::vector<WGPUBindGroupEntry> entries {};
        std::ranges::transform(spec.Entries, std::back_inserter(entries), [](BindGroupEntry const& entry) {
            WGPUBindGroupEntry wgpu_entry {
                .nextInChain = nullptr,
                .binding = entry.Binding,
                .buffer = nullptr,
                .offset = 0,
                .size = 0,
                .sampler = nullptr,
                .textureView = nullptr,
            };

            std::visit(overloaded {
                           [&](BufferBinding const& buffer_binding) {
                               wgpu_entry.buffer = CAST(WGPUBuffer, buffer_binding.TargetBuffer.Handle);
                               wgpu_entry.offset = buffer_binding.Offset;
                               wgpu_entry.size = buffer_binding.Size;
                           },
                           [&](Sampler const& sampler) {
                               wgpu_entry.sampler = CAST(WGPUSampler, sampler.Handle);
                           },
                           [&](TextureView const& view) {
                               wgpu_entry.textureView = CAST(WGPUTextureView, view.Handle);
                           },
                           [](auto&&) {} },
                entry.Resource);

            return wgpu_entry;
        });

        WGPUBindGroupDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Bind Group"sv).data(),
            .layout = CAST(WGPUBindGroupLayout, layout.Handle),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bind_group = wgpuDeviceCreateBindGroup(CAST(WGPUDevice, Handle), &desc);
        return BindGroup { bind_group };
    }

    auto Device::CreateBindGroupLayout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout
    {
        std::vector<WGPUBindGroupLayoutEntry> entries {};
        std::ranges::transform(spec.Entries, std::back_inserter(entries), [](BindGroupLayoutEntry const& entry) {
            WGPUBindGroupLayoutEntry wgpu_entry {
                .nextInChain = nullptr,
                .binding = entry.Binding,
                .visibility = ToWGPU(entry.Visibility),
            };

            std::visit(overloaded {
                           [&](BindingType::Buffer const& buffer) {
                               wgpu_entry.buffer = {
                                   .nextInChain = nullptr,
                                   .hasDynamicOffset = buffer.HasDynamicOffset,
                               };

                               wgpu_entry.buffer.type = std::visit(overloaded {
                                                                       [&](BufferBindingType::Uniform const&) {
                                                                           return WGPUBufferBindingType_Uniform;
                                                                       },
                                                                       [&](BufferBindingType::Storage const& storage) {
                                                                           if (storage.ReadOnly) {
                                                                               return WGPUBufferBindingType_ReadOnlyStorage;
                                                                           }
                                                                           return WGPUBufferBindingType_Storage;
                                                                       },
                                                                       [](auto&&) { return WGPUBufferBindingType_Undefined; } },
                                   buffer.Type);

                               if (buffer.MinBindingSize) {
                                   wgpu_entry.buffer.minBindingSize = *buffer.MinBindingSize;
                               }
                           },
                           [&](BindingType::Sampler const& sampler) {
                               wgpu_entry.sampler.type = ToWGPU(sampler.Type);
                           },
                           [&](BindingType::Texture const& texture) {
                               wgpu_entry.texture = {
                                   .nextInChain = nullptr,
                                   .viewDimension = ToWGPU(texture.ViewDimension),
                                   .multisampled = texture.MultiSampled,
                               };
                               wgpu_entry.texture.sampleType = std::visit(overloaded {
                                                                              [](TextureSampleType::Float const& flt) {
                                                                                  if (flt.Filterable) {
                                                                                      return WGPUTextureSampleType_Float;
                                                                                  }
                                                                                  return WGPUTextureSampleType_UnfilterableFloat;
                                                                              },
                                                                              [](TextureSampleType::Depth const&) {
                                                                                  return WGPUTextureSampleType_Depth;
                                                                              },
                                                                              [](TextureSampleType::SInt const&) {
                                                                                  return WGPUTextureSampleType_Sint;
                                                                              },
                                                                              [](TextureSampleType::UInt const&) {
                                                                                  return WGPUTextureSampleType_Uint;
                                                                              },
                                                                              [](auto&&) { return WGPUTextureSampleType_Undefined; } },
                                   texture.SampleType);
                           },
                           [&](BindingType::StorageTexture const& storage_texture) {
                               wgpu_entry.storageTexture = {
                                   .nextInChain = nullptr,
                                   .access = ToWGPU(storage_texture.Access),
                                   .format = ToWGPU(storage_texture.Format),
                                   .viewDimension = ToWGPU(storage_texture.ViewDimension),
                               };
                           },
                           [](BindingType::AccelerationStructure&&) {
                               PANIC("TODO!");
                           },
                           [](auto&&) {},
                       },
                entry.Type);

            return wgpu_entry;
        });

        WGPUBindGroupLayoutDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Bind Group"sv).data(),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bg_layout = wgpuDeviceCreateBindGroupLayout(CAST(WGPUDevice, Handle), &desc);
        return BindGroupLayout { bg_layout };
    }

    auto Device::CreateQuerySet(QuerySetSpec const& spec) const -> QuerySet
    {
        // This is declared outside the visit lambdas because it will be reffered
        // to, by a pointer from the WGPUQuerySetDescriptorExtras struct.
        std::vector<WGPUPipelineStatisticName> pipeline_statistic_names {};
        pipeline_statistic_names.reserve(5);

        WGPUQuerySetDescriptorExtras extras {
            .chain = {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_QuerySetDescriptorExtras) },
        };

        WGPUQuerySetDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("QuerySet").data.ptr,
            .count = spec.Count,
        };

        std::visit(overloaded {
                       [&desc](QueryType::Occlusion const&) {
                           desc.type = WGPUQueryType_Occlusion;
                       },
                       [&desc](QueryType::Timestamp const&) {
                           desc.type = WGPUQueryType_Timestamp;
                       },
                       [&desc, &extras, &pipeline_statistic_names](PipelineStatisticNameFlags const& flags) {
                           if (flags.test(PipelineStatisticName::ClipperInvocations)) {
                               pipeline_statistic_names.push_back(WGPUPipelineStatisticName_ClipperInvocations);
                           }
                           if (flags.test(PipelineStatisticName::ClipperPrimitivesOut)) {
                               pipeline_statistic_names.push_back(WGPUPipelineStatisticName_ClipperPrimitivesOut);
                           }
                           if (flags.test(PipelineStatisticName::ComputeShaderInvocations)) {
                               pipeline_statistic_names.push_back(WGPUPipelineStatisticName_ComputeShaderInvocations);
                           }
                           if (flags.test(PipelineStatisticName::FragmentShaderInvocations)) {
                               pipeline_statistic_names.push_back(WGPUPipelineStatisticName_FragmentShaderInvocations);
                           }
                           if (flags.test(PipelineStatisticName::VertexShaderInvocations)) {
                               pipeline_statistic_names.push_back(WGPUPipelineStatisticName_VertexShaderInvocations);
                           }

                           desc.type = CAST(WGPUQueryType, WGPUNativeQueryType_PipelineStatistics);
                           desc.nextInChain = &extras.chain;
                           extras.pipelineStatistics = pipeline_statistic_names.data();
                           extras.pipelineStatisticCount = pipeline_statistic_names.size();
                       },
                   },
            spec.Type);

        auto set = wgpuDeviceCreateQuerySet(CAST(WGPUDevice, Handle), &desc);
        return QuerySet(set, spec);
    }

    auto Device::CreateShaderModule(ShaderModuleSpec const& spec) const -> ShaderModule
    {
        auto desc = std::visit(overloaded {
                                   [](WGSLShader const& wgsl) {
                                       WGPUShaderModuleWGSLDescriptor wgsl_descriptor {
                                           .chain = {
                                               .next = nullptr,
                                               .sType = WGPUSType_ShaderModuleWGSLDescriptor,
                                           },
                                           .code = wgsl.Source.data(),
                                       };
                                       WGPUShaderModuleDescriptor desc {
                                           .nextInChain = &wgsl_descriptor.chain,
                                           .label = "Shader",
                                           .hintCount = 0,
                                           .hints = nullptr,
                                       };
                                       return desc;
                                   },
                                   [](SPIRVShader const& spirv) {
                                       WGPUShaderModuleSPIRVDescriptor spirv_descriptor {
                                           .chain = {
                                               .next = nullptr,
                                               .sType = WGPUSType_ShaderModuleSPIRVDescriptor,
                                           },
                                           .codeSize = CAST(u32, spirv.Binary.size_bytes()),
                                           .code = spirv.Binary.data(),
                                       };
                                       WGPUShaderModuleDescriptor desc {
                                           .nextInChain = &spirv_descriptor.chain,
                                           .label = "Shader",
                                           .hintCount = 0,
                                           .hints = nullptr,
                                       };

                                       return desc;
                                   },
                                   [](auto&&) {},
                               },
            spec.Type);

        auto module = wgpuDeviceCreateShaderModule(CAST(WGPUDevice, Handle), &desc);

        return ShaderModule { module, spec };
    }

#ifdef WGPU_DEV
    auto Device::CreateShaderModuleSpirV(SpirVShaderSpec const& spec) const -> ShaderModule
    {
        WGPUShaderModuleDescriptorSpirV desc;
        desc.label = spec.Label.ValueOr("SpirVShaderModule"sv).data();
        desc.sourceSize = CAST(u32, spec.Data.size());
        desc.source = spec.Data.data();
        auto module = wgpuDeviceCreateShaderModuleSpirV(CAST(WGPUDevice, Handle), &desc);
        return ShaderModule { module };
    }
#endif

    auto Device::CreatePipelineLayout(PipelineLayoutSpec const& spec) const -> PipelineLayout
    {
        std::vector<WGPUBindGroupLayout> layouts {};
        std::ranges::transform(spec.BindGroupLayouts, std::back_inserter(layouts), [](BindGroupLayout const& bind_group_layout) {
            return CAST(WGPUBindGroupLayout, bind_group_layout.Handle);
        });

        WGPUPipelineLayoutDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Pipeline Layout"sv).data(),
            .bindGroupLayoutCount = CAST(u32, layouts.size()),
            .bindGroupLayouts = layouts.data(),
        };

        auto layout = wgpuDeviceCreatePipelineLayout(CAST(WGPUDevice, Handle), &desc);
        return PipelineLayout { layout };
    }

    auto Device::CreateRenderPipeline(ShaderModule const& vert_module, ShaderModule const& frag_module, RenderPipelineSpec const& spec) const -> RenderPipeline
    {
        std::vector<WGPUVertexBufferLayout> vertex_buffer_layouts {};
        std::vector<std::vector<WGPUVertexAttribute>> attributes {};
        attributes.resize(spec.Vertex.AttributeLayouts.size());

        size_t attribute_index = 0;
        std::ranges::transform(spec.Vertex.AttributeLayouts, std::back_inserter(vertex_buffer_layouts), [&](VertexBufferLayout const& layout) {
            std::ranges::transform(layout.Attributes, std::back_inserter(attributes[attribute_index]), [](VertexAttribute const& attribute) {
                return WGPUVertexAttribute {
                    .format = ToWGPU(attribute.Type),
                    .offset = attribute.Offset,
                    .shaderLocation = attribute.ShaderLocation,
                };
            });

            WGPUVertexBufferLayout wgpu_layout {
                .arrayStride = layout.ArrayStride,
                .stepMode = ToWGPU(layout.StepMode),
                .attributeCount = CAST(u32, attributes[attribute_index].size()),
                .attributes = attributes[attribute_index].data(),
            };

            attribute_index++;
            return wgpu_layout;
        });

        WGPUVertexState vertex {
            .nextInChain = nullptr,
            .module = vert_module.As<WGPUShaderModule>(),
            .entryPoint = spec.VertexEntryPointOverride.ValueOr(vert_module.Spec.VertexEntryPoint).data(),
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = CAST(u32, vertex_buffer_layouts.size()),
            .buffers = vertex_buffer_layouts.data(),
        };

        WGPUPrimitiveState primitive {
            .nextInChain = nullptr,
            .topology = ToWGPU(spec.Primitive.Topology),
            .stripIndexFormat = ToWGPU(spec.Primitive.StripIndexFormat.ValueOr(IndexFormat::Undefined)),
            .frontFace = ToWGPU(spec.Primitive.FrontFace),
            .cullMode = ToWGPU(spec.Primitive.Cull),
        };

        if (spec.Primitive.StripIndexFormat) {
            primitive.stripIndexFormat = ToWGPU(*spec.Primitive.StripIndexFormat);
        }

        WGPUMultisampleState multisample {
            .nextInChain = nullptr,
            .count = spec.MultiSample.Count,
            .mask = spec.MultiSample.Mask,
            .alphaToCoverageEnabled = spec.MultiSample.AlphaToCoverageEnabled,
        };

        std::vector<WGPUColorTargetState> color_targets {};
        // We need to save these here because the color
        // target state gets a pointer to the blend state.
        std::vector<WGPUBlendState> blends {};
        WGPUFragmentState fragment {};

        if (spec.Fragment.HasValue()) {
            fragment.module = frag_module.As<WGPUShaderModule>();
            fragment.entryPoint = spec.FragmentEntryPointOverride.ValueOr(frag_module.Spec.FragmentEntryPoint).data();
            fragment.constantCount = 0;
            fragment.constants = nullptr;
            // Resize to a max of spec.Fragment.Targets.size() to prevent reallocations
            // and dangling pointers.
            blends.reserve(spec.Fragment->Targets.size());

            std::ranges::transform(spec.Fragment->Targets, std::back_inserter(color_targets), [&blends](ColorTargetState const& state) {
                WGPUColorTargetState wgpu_state {
                    .nextInChain = nullptr,
                    .format = ToWGPU(state.Format),
                    .blend = nullptr,
                    .writeMask = ToWGPU(state.WriteMask),
                };

                if (auto b = state.Blend) {
                    auto& blend = blends.emplace_back();
                    blend = {
                        .color = {
                            .operation = ToWGPU(b->Color.Operation),
                            .srcFactor = ToWGPU(b->Color.SrcFactor),
                            .dstFactor = ToWGPU(b->Color.DstFactor) },
                        .alpha = { .operation = ToWGPU(b->Alpha.Operation), .srcFactor = ToWGPU(b->Alpha.SrcFactor), .dstFactor = ToWGPU(b->Alpha.DstFactor) }
                    };

                    wgpu_state.blend = &blend;
                }

                return wgpu_state;
            });

            fragment.targetCount = CAST(u32, color_targets.size());
            fragment.targets = color_targets.data();
        }

        WGPURenderPipelineDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Render Pipeline"sv).data(),
            .vertex = vertex,
            .primitive = primitive,
            .depthStencil = nullptr,
            .multisample = multisample,
            .fragment = spec.Fragment ? &fragment : nullptr,
        };
        if (spec.Layout.HasValue()) {
            desc.layout = spec.Layout->As<WGPUPipelineLayout>();
        }

        WGPUDepthStencilState depth_stencil;
        if (spec.DepthStencil) {
            depth_stencil = {
                .nextInChain = nullptr,
                .format = ToWGPU(spec.DepthStencil->Format),
                .depthWriteEnabled = spec.DepthStencil->DepthWriteEnabled,
                .depthCompare = ToWGPU(spec.DepthStencil->DepthCompare),
                .stencilFront = {
                    .compare = ToWGPU(spec.DepthStencil->Stencil.Front.CompareFunc),
                    .failOp = ToWGPU(spec.DepthStencil->Stencil.Front.FailOp),
                    .depthFailOp = ToWGPU(spec.DepthStencil->Stencil.Front.DepthFailOp),
                    .passOp = ToWGPU(spec.DepthStencil->Stencil.Front.PassOp),
                },
                .stencilBack = {
                    .compare = ToWGPU(spec.DepthStencil->Stencil.Back.CompareFunc),
                    .failOp = ToWGPU(spec.DepthStencil->Stencil.Back.FailOp),
                    .depthFailOp = ToWGPU(spec.DepthStencil->Stencil.Back.DepthFailOp),
                    .passOp = ToWGPU(spec.DepthStencil->Stencil.Back.PassOp),
                },
                .stencilReadMask = spec.DepthStencil->Stencil.ReadMask,
                .stencilWriteMask = spec.DepthStencil->Stencil.WriteMask,
                .depthBias = spec.DepthStencil->Bias.Constant,
                .depthBiasSlopeScale = spec.DepthStencil->Bias.SlopeScale,
                .depthBiasClamp = spec.DepthStencil->Bias.Clamp
            };
            desc.depthStencil = &depth_stencil;
        }

        auto pipeline = wgpuDeviceCreateRenderPipeline(CAST(WGPUDevice, Handle), &desc);
        return RenderPipeline { pipeline };
    }

    std::mutex QueueMutex {};

    void Device::SubmitCommandBuffer(CommandBuffer cmd) const
    {
        ZoneScoped;
        std::lock_guard lock(QueueMutex);

        WGPUCommandBuffer cmds[] = {
            CAST(WGPUCommandBuffer, cmd.handle),
        };
        wgpuQueueSubmit(CAST(WGPUQueue, Queue), 1, cmds);
    }

    void Device::WriteTexture(
        Texture const& texture,
        void const* data,
        size_t data_size,
        Vector2 const& origin,
        Vector2 const& size,
        u32 bytes_per_pixel,
        u32 mip_level) const
    {
        WGPUImageCopyTexture copy_texture {
            .nextInChain = nullptr,
            .texture = CAST(WGPUTexture, texture.Handle),
            .mipLevel = mip_level,
            .origin = {
                .x = CAST(u32, origin.x),
                .y = CAST(u32, origin.y),
                .z = 0,
            },
            .aspect = WGPUTextureAspect_All,
        };

        WGPUTextureDataLayout layout {
            .nextInChain = nullptr,
            .offset = 0,
            .bytesPerRow = CAST(u32, size.x * bytes_per_pixel),
            .rowsPerImage = CAST(u32, size.y),
        };

        WGPUExtent3D texture_size {
            .width = CAST(u32, texture.Spec.Size.x),
            .height = CAST(u32, texture.Spec.Size.y),
            .depthOrArrayLayers = 1,
        };
        wgpuQueueWriteTexture(CAST(WGPUQueue, Queue), &copy_texture, data, data_size, &layout, &texture_size);
        wgpuDevicePoll(As<WGPUDevice>(), true, nullptr);
    }

    void Device::Release()
    {
        wgpuDeviceRelease(As<WGPUDevice>());
    }

    void Device::WriteBuffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const
    {
        wgpuQueueWriteBuffer(CAST(WGPUQueue, Queue), CAST(WGPUBuffer, buffer.Handle), offset, data, size);
    }

    // void Device::SetErrorCallback(ErrorFn const& function)
    // {
    //     m_Function = std::move(function);
    //
    //     wgpuDeviceSetUncapturedErrorCallback(
    //         CAST(WGPUDevice, Handle), [](WGPUErrorType type, char const* message, void* userdata) {
    //             auto self = CAST(Device*, userdata);
    //
    //             if (self->m_Function)
    //                 self->m_Function(FromWgpu(type), std::string_view(message));
    //         },
    //         this);
    // }

    auto Adapter::RequestDevice(DeviceSpec const& spec) -> Device
    {
        std::vector<WGPUFeatureName> features {};
        for (auto const& feature : spec.RequiredFeatures) {
            features.push_back(ToWGPU(feature));
        }

        WGPUDeviceDescriptor desc {
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Device").data.ptr,
            .requiredFeatureCount = features.size(),
            .requiredFeatures = features.data(),
            .defaultQueue = {
                .nextInChain = nullptr,
                .label = "Default Queue" },
            .deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void*) {
                PANIC("!DEVICE LOST!: \n\tREASON: {}\n\tMESSAGE: {}", magic_enum::enum_name(reason), message);
            },
            .deviceLostUserdata = this,
            .uncapturedErrorCallbackInfo = {
                .callback = [](WGPUErrorType type, char const* message, void*) {
                    LOG_ERRORF("!DEVICE ERROR!\n\tTYPE: {}\n\tMESSAGE: {}", magic_enum::enum_name(type), message);
                    PANIC("DEVICE ERROR");
                },
                .userdata = this,
            },
        };

        auto device = request_device_sync(CAST(WGPUAdapter, Handle), &desc);

        return Device { device };
    }

    bool Adapter::HasFeature(Feature feature) const
    {
        return wgpuAdapterHasFeature(CAST(WGPUAdapter, Handle), ToWGPU(feature));
    }

    void Adapter::Release() const
    {
        wgpuAdapterRelease(CAST(WGPUAdapter, Handle));
    }

    void Surface::Release() const
    {
        wgpuSurfaceRelease(CAST(WGPUSurface, Handle));
    }

    void Surface::Configure(Device const& device, Adapter adapter, Config const& config)
    {
        WGPUSurfaceCapabilities caps {};
        wgpuSurfaceGetCapabilities(CAST(WGPUSurface, Handle), CAST(WGPUAdapter, adapter.Handle), &caps);
        VERIFY(caps.formatCount >= 1, "Surface without formats?!!!");
        LOG_DEBUGF("Configuring surface with format: {}", magic_enum::enum_name(caps.formats[0]));

        WGPUSurfaceConfigurationExtras extras {
            .chain = WGPUChainedStruct {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_SurfaceConfigurationExtras),
            },
            .desiredMaximumFrameLatency = 2
        };
        WGPUSurfaceConfiguration conf {
            .nextInChain = &extras.chain,
            .device = CAST(WGPUDevice, device.Handle),
            .format = caps.formats[0], // TODO: Check this properly
            .usage = WGPUTextureUsage_RenderAttachment,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
            .alphaMode = WGPUCompositeAlphaMode_Auto,
            .width = CAST(u32, config.Size.x),
            .height = CAST(u32, config.Size.y),
            .presentMode = ToWGPU(config.Mode),
        };
        wgpuSurfaceConfigure(CAST(WGPUSurface, Handle), &conf);

        Format = from_wgpu(caps.formats[0]);
    }

    auto Surface::GetNextView() const -> Result<TextureView, Error>
    {
        WGPUSurfaceTexture texture;
        {
            ZoneScopedN("GetCurrentTexture");
            wgpuSurfaceGetCurrentTexture(CAST(WGPUSurface, Handle), &texture);
        }

        if (texture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
            switch (texture.status) {
            case WGPUSurfaceGetCurrentTextureStatus_Timeout:
                return Error::Timeout;
            case WGPUSurfaceGetCurrentTextureStatus_Outdated:
                return Error::Outdated;
            case WGPUSurfaceGetCurrentTextureStatus_Lost:
                return Error::Lost;
            case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
                return Error::OutOfMemory;
            default:
                PANIC("Unknown GetCurrentTexture error: {}", magic_enum::enum_name(texture.status));
            }
        }

        WGPUTextureViewDescriptor view_descriptor;
        view_descriptor.nextInChain = nullptr;
        view_descriptor.label = "Surface texture view";
        view_descriptor.format = wgpuTextureGetFormat(texture.texture);
        view_descriptor.dimension = WGPUTextureViewDimension_2D;
        view_descriptor.baseMipLevel = 0;
        view_descriptor.mipLevelCount = 1;
        view_descriptor.baseArrayLayer = 0;
        view_descriptor.arrayLayerCount = 1;
        view_descriptor.aspect = WGPUTextureAspect_All;
        WGPUTextureView target_view = wgpuTextureCreateView(texture.texture, &view_descriptor);
        return TextureView { target_view };
    }

    void Surface::Present() const
    {
        ZoneScoped;
        wgpuSurfacePresent(CAST(WGPUSurface, Handle));
    }

    Instance Instance::Create(InstanceSpec const& spec)
    {
        Instance instance;
        WGPUInstanceExtras instance_extras {
            .chain = {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_InstanceExtras),
            },
            .backends = ToWGPU(spec.Backend),
            .flags = WGPUInstanceFlag_Debug,
        };

        WGPUInstanceDescriptor desc {
            .nextInChain = &instance_extras.chain,
        };
        instance.Handle = wgpuCreateInstance(&desc);
        return instance;
    }

    void Instance::Release() const
    {
        wgpuInstanceRelease(CAST(WGPUInstance, Handle));
    }

    auto Instance::GetSurface(Window const* window) const -> Surface
    {
        auto glfw_window = CAST(GLFWwindow*, window->NativeHandle());
        auto surface = glfwGetWGPUSurface(CAST(WGPUInstance, Handle), glfw_window);
        return Surface { surface };
    }

    GlobalReport Instance::GenerateGlobalReport() const
    {
        WGPUGlobalReport report {};
        wgpuGenerateReport(CAST(WGPUInstance, Handle), &report);

        GlobalReport my_report {
            .Adapters = {
                .NumAllocated = report.vulkan.adapters.numAllocated,
                .NumKeptFromUser = report.vulkan.adapters.numKeptFromUser,
                .NumReleasedFromUser = report.vulkan.adapters.numReleasedFromUser,
                .NumError = report.vulkan.adapters.numError,
                .ElementSize = report.vulkan.adapters.elementSize },
            .Devices = { .NumAllocated = report.vulkan.devices.numAllocated, .NumKeptFromUser = report.vulkan.devices.numKeptFromUser, .NumReleasedFromUser = report.vulkan.devices.numReleasedFromUser, .NumError = report.vulkan.devices.numError, .ElementSize = report.vulkan.devices.elementSize },
            .Queues = { .NumAllocated = report.vulkan.queues.numAllocated, .NumKeptFromUser = report.vulkan.queues.numKeptFromUser, .NumReleasedFromUser = report.vulkan.queues.numReleasedFromUser, .NumError = report.vulkan.queues.numError, .ElementSize = report.vulkan.queues.elementSize },
            .PipelineLayouts = { .NumAllocated = report.vulkan.pipelineLayouts.numAllocated, .NumKeptFromUser = report.vulkan.pipelineLayouts.numKeptFromUser, .NumReleasedFromUser = report.vulkan.pipelineLayouts.numReleasedFromUser, .NumError = report.vulkan.pipelineLayouts.numError, .ElementSize = report.vulkan.pipelineLayouts.elementSize },
            .ShaderModules = { .NumAllocated = report.vulkan.shaderModules.numAllocated, .NumKeptFromUser = report.vulkan.shaderModules.numKeptFromUser, .NumReleasedFromUser = report.vulkan.shaderModules.numReleasedFromUser, .NumError = report.vulkan.shaderModules.numError, .ElementSize = report.vulkan.shaderModules.elementSize },
            .BindGroupLayouts = { .NumAllocated = report.vulkan.bindGroupLayouts.numAllocated, .NumKeptFromUser = report.vulkan.bindGroupLayouts.numKeptFromUser, .NumReleasedFromUser = report.vulkan.bindGroupLayouts.numReleasedFromUser, .NumError = report.vulkan.bindGroupLayouts.numError, .ElementSize = report.vulkan.bindGroupLayouts.elementSize },
            .BindGroups = { .NumAllocated = report.vulkan.bindGroups.numAllocated, .NumKeptFromUser = report.vulkan.bindGroups.numKeptFromUser, .NumReleasedFromUser = report.vulkan.bindGroups.numReleasedFromUser, .NumError = report.vulkan.bindGroups.numError, .ElementSize = report.vulkan.bindGroups.elementSize },
            .CommandBuffers = { .NumAllocated = report.vulkan.commandBuffers.numAllocated, .NumKeptFromUser = report.vulkan.commandBuffers.numKeptFromUser, .NumReleasedFromUser = report.vulkan.commandBuffers.numReleasedFromUser, .NumError = report.vulkan.commandBuffers.numError, .ElementSize = report.vulkan.commandBuffers.elementSize },
            .RenderBundles = { .NumAllocated = report.vulkan.renderBundles.numAllocated, .NumKeptFromUser = report.vulkan.renderBundles.numKeptFromUser, .NumReleasedFromUser = report.vulkan.renderBundles.numReleasedFromUser, .NumError = report.vulkan.renderBundles.numError, .ElementSize = report.vulkan.renderBundles.elementSize },
            .RenderPipelines = { .NumAllocated = report.vulkan.renderPipelines.numAllocated, .NumKeptFromUser = report.vulkan.renderPipelines.numKeptFromUser, .NumReleasedFromUser = report.vulkan.renderPipelines.numReleasedFromUser, .NumError = report.vulkan.renderPipelines.numError, .ElementSize = report.vulkan.renderPipelines.elementSize },
            .ComputePipelines = { .NumAllocated = report.vulkan.computePipelines.numAllocated, .NumKeptFromUser = report.vulkan.computePipelines.numKeptFromUser, .NumReleasedFromUser = report.vulkan.computePipelines.numReleasedFromUser, .NumError = report.vulkan.computePipelines.numError, .ElementSize = report.vulkan.computePipelines.elementSize },
            .QuerySets = { .NumAllocated = report.vulkan.querySets.numAllocated, .NumKeptFromUser = report.vulkan.querySets.numKeptFromUser, .NumReleasedFromUser = report.vulkan.querySets.numReleasedFromUser, .NumError = report.vulkan.querySets.numError, .ElementSize = report.vulkan.querySets.elementSize },
            .Buffers = { .NumAllocated = report.vulkan.buffers.numAllocated, .NumKeptFromUser = report.vulkan.buffers.numKeptFromUser, .NumReleasedFromUser = report.vulkan.buffers.numReleasedFromUser, .NumError = report.vulkan.buffers.numError, .ElementSize = report.vulkan.buffers.elementSize },
            .Textures = { .NumAllocated = report.vulkan.textures.numAllocated, .NumKeptFromUser = report.vulkan.textures.numKeptFromUser, .NumReleasedFromUser = report.vulkan.textures.numReleasedFromUser, .NumError = report.vulkan.textures.numError, .ElementSize = report.vulkan.textures.elementSize },
            .TextureViews = { .NumAllocated = report.vulkan.textureViews.numAllocated, .NumKeptFromUser = report.vulkan.textureViews.numKeptFromUser, .NumReleasedFromUser = report.vulkan.textureViews.numReleasedFromUser, .NumError = report.vulkan.textureViews.numError, .ElementSize = report.vulkan.textureViews.elementSize },
            .Samplers = { .NumAllocated = report.vulkan.samplers.numAllocated, .NumKeptFromUser = report.vulkan.samplers.numKeptFromUser, .NumReleasedFromUser = report.vulkan.samplers.numReleasedFromUser, .NumError = report.vulkan.samplers.numError, .ElementSize = report.vulkan.samplers.elementSize }
        };
        return my_report;
    }
    bool IsHDR(TextureFormat format)
    {
        switch (format) {
            using enum TextureFormat;
        case RGBA16Float:
        case RGBA32Float:
            return true;
        default:
            return false;
        }
    }

    auto Instance::GetAdapter(Surface surface, AdapterOptions const& opt) const -> Adapter
    {
        WGPURequestAdapterOptions options {
            .nextInChain = nullptr,
            .compatibleSurface = CAST(WGPUSurface, surface.Handle),
            .powerPreference = ToWGPU(opt.PowerPreference),
        };
        auto adapter = request_adapter_sync(CAST(WGPUInstance, Handle), &options);
        return Adapter { adapter };
    }

    void CommandBuffer::Release() const
    {
        ZoneScoped;
        wgpuCommandBufferRelease(CAST(WGPUCommandBuffer, handle));
    }
}
