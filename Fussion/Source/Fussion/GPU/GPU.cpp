#include "GPU.h"

#include "EnumConversions.h"
#include "FussionPCH.h"
#include "glfw3webgpu.h"
#include "Utils.h"

#include <GLFW/glfw3.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <webgpu/wgpu.h>
#include <webgpu/webgpu.h>

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
        ShaderModule shader{};
        RenderPipeline pipeline{};
        BindGroupLayout layout{};
        BindGroup bind_group{};
        Sampler sampler{};
        Texture render_texture{};
        Texture target_texture{};
        u32 mip_levels{};
        std::vector<TextureView> views{};

        void Initalize(Device const& device, Texture& texture)
        {
            target_texture = texture;
            {
                Vector2 size = { texture.spec.size.x, texture.spec.size.y };
                render_texture = device.create_texture({
                    .usage = TextureUsage::CopySrc | TextureUsage::RenderAttachment | TextureUsage::CopyDst,
                    .dimension = TextureDimension::D2,
                    .size = { texture.spec.size.x, texture.spec.size.y, 1 },
                    .format = texture.spec.format,
                    .sample_count = 1,
                    .aspect = TextureAspect::All,
                    .generate_mip_maps = true,
                });
                render_texture.initialize_view();

                auto encoder = device.create_command_encoder();
                encoder.copy_texture_to_texture(texture, render_texture, size);
                auto cmd = encoder.finish();
                device.submit_command_buffer(cmd);
            }
            mip_levels = texture.mip_level_count;
            ShaderModuleSpec shader_spec{
                .label = "MipMap Generator"sv,
                .type = GPU::WGSLShader{
                    .source = MipMapGeneratorSource,
                },
                .vertex_entry_point = "vs_main",
                .fragment_entry_point = "fs_main",
            };
            shader = device.create_shader_module(shader_spec);

            RenderPipelineSpec spec{
                .label = "fuck"sv,
                .layout = None(),
                .primitive = {
                    .topology = PrimitiveTopology::TriangleList,
                    .strip_index_format = None(),
                    .front_face = FrontFace::Ccw,
                    .cull = Face::None,
                },
                .depth_stencil = None(),
                .multi_sample = MultiSampleState::get_default(),
                .fragment = FragmentStage{
                    .targets = {
                        ColorTargetState{
                            .format = texture.spec.format,
                            .blend = BlendState::get_default(),
                            .write_mask = ColorWrite::All,
                        }
                    },
                },
            };
            spec.primitive.topology = PrimitiveTopology::TriangleStrip;

            pipeline = device.create_render_pipeline(shader, shader, spec);

            layout = pipeline.bind_group_layout(0);

            SamplerSpec sampler_spec{
                .lod_min_clamp = 0.f,
                .lod_max_clamp = 0.f,
                .anisotropy_clamp = 2,
            };
            sampler = device.create_sampler(sampler_spec);

            std::array entries = {
                BindGroupEntry{
                    .binding = 0,
                    .resource = texture.view,
                },
                BindGroupEntry{
                    .binding = 1,
                    .resource = sampler
                },
            };
            BindGroupSpec bg_spec{
                .label = "fuck2"sv,
                .entries = entries,
            };

            bind_group = device.create_bind_group(layout, bg_spec);

            // NOTE: When this is called from another thread logging
            //       stuff here causes problems like segfaults and
            //       other race condition fun stuff.

            for (u32 i = 1; i < mip_levels; ++i) {
                views.emplace_back(render_texture.create_view({
                    .label = "View"sv,
                    .usage = render_texture.spec.usage,
                    .dimension = TextureViewDimension::D2,
                    .format = render_texture.spec.format,
                    .base_mip_level = i,
                    .mip_level_count = 1,
                    .base_array_layer = 0,
                    .array_layer_count = 1,
                    .aspect = render_texture.spec.aspect
                }));
            }
        }

        void Process(Device const& device)
        {
            auto encoder = device.create_command_encoder("MipMap Generation");
            u32 i = 1;
            Vector2 size = render_texture.spec.size;
            for (auto& view : views) {
                if (size.x > 1)
                    size.x = CAST(f32, CAST(u32, size.x) / 2);
                if (size.y > 1)
                    size.y = CAST(f32, CAST(u32, size.y) / 2);

                std::array colors = {
                    RenderPassColorAttachment{
                        .view = view,
                        .load_op = LoadOp::Clear,
                        .store_op = StoreOp::Store,
                        .clear_color = Color::Magenta,
                    },
                };
                RenderPassSpec spec{
                    .color_attachments = colors
                };
                auto rp = encoder.begin_rendering(spec);

                rp.set_pipeline(pipeline);
                rp.set_bind_group(bind_group, 0);
                rp.draw({ 0, 6 }, { 0, 1 });
                rp.end();
                rp.release();

                encoder.copy_texture_to_texture(render_texture, target_texture, size, i, i);
                ++i;
            }

            auto cmd = encoder.finish();
            device.submit_command_buffer(cmd);

            encoder.release();
            for (auto& view : views) {
                view.release();
            }
            pipeline.release();
            sampler.release();
            bind_group.release();
            shader.release();
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

    void QuerySet::release()
    {
        wgpuQuerySetRelease(as<WGPUQuerySet>());
    }

    void Sampler::release()
    {
        wgpuSamplerRelease(as<WGPUSampler>());
    }

    Buffer::Buffer(HandleT handle, BufferSpec const& spec): GPUHandle(handle, spec)
    {
        if (spec.mapped)
            m_map_state = MapState::Mapped;
    }

    auto Buffer::size() const -> u64
    {
        return wgpuBufferGetSize(CAST(WGPUBuffer, handle));
    }

    auto Buffer::slice(u32 start, u32 size) -> BufferSlice
    {
        return BufferSlice(*this, start, size);
    }

    auto Buffer::slice() -> BufferSlice
    {
        return BufferSlice(*this, 0, size());
    }

    auto Buffer::map_state() const -> MapState
    {
        return m_map_state;
    }

    void Buffer::unmap()
    {
        wgpuBufferUnmap(as<WGPUBuffer>());
        m_map_state = MapState::Unmapped;
    }

    void Buffer::release()
    {
        wgpuBufferRelease(CAST(WGPUBuffer, handle));
    }

    void Buffer::force_map_state(MapState state)
    {
        m_map_state = state;

    }

    BufferSlice::BufferSlice(Buffer& buffer, u32 start, u32 size)
        : backing_buffer(&buffer)
          , start(start)
          , size(size) {}

    BufferSlice::BufferSlice(Buffer& buffer)
        : backing_buffer(&buffer)
          , start(0)
          , size(buffer.size()) {}

    auto BufferSlice::mapped_range() -> void*
    {
        return wgpuBufferGetMappedRange(backing_buffer->as<WGPUBuffer>(), start, size);
    }

    void BufferSlice::map_async(MapModeFlags map_mode, AsyncMapCallback const& callback) const
    {
        struct UserData {
            Buffer* buffer;
            AsyncMapCallback callback;
        };
        backing_buffer->m_map_state = MapState::Pending;

        wgpuBufferMapAsync(backing_buffer->as<WGPUBuffer>(), to_wgpu(map_mode), start, size, [](WGPUBufferMapAsyncStatus status, void* userdata) {
            auto user_data = CAST(UserData*, userdata);
            if (status == WGPUBufferMapAsyncStatus_Success) {
                user_data->buffer->m_map_state = MapState::Mapped;
                user_data->callback();
            } else {
                LOG_ERRORF("Could not map buffer: {}", magic_enum::enum_name(status));
            }

            delete user_data;
        }, new UserData(backing_buffer, callback));
    }

    void TextureView::release()
    {
        wgpuTextureViewRelease(CAST(WGPUTextureView, handle));
    }

    void Texture::initialize_view(u32 array_count)
    {
        view = create_view({
            .label = "View"sv,
            .usage = spec.usage,
            .dimension = array_count == 1 ? TextureViewDimension::D2 : TextureViewDimension::D2_Array, // TODO: Make configurable
            .format = spec.format,
            .base_mip_level = 0, // TODO: Make configurable
            .mip_level_count = mip_level_count,
            .base_array_layer = 0, // TODO: Make configurable
            .array_layer_count = array_count, // TODO: Make configurable
            .aspect = spec.aspect // TODO: Make configurable
        });
    }

    void Texture::generate_mipmaps(Device const& device)
    {
        if (spec.generate_mip_maps && mip_level_count > 1) {
            MipMapPipeline mmp;
            mmp.Initalize(device, *this);

            //Utils::RenderDoc::StartCapture();
            mmp.Process(device);
            //Utils::RenderDoc::EndCapture();
        }
    }

    TextureView Texture::create_view(TextureViewSpec const& spec) const
    {
        WGPUTextureViewDescriptor texture_view_descriptor{
            .nextInChain = nullptr,
            .label = spec.label.value_or("View"sv).data(),
            .format = to_wgpu(spec.format),
            .dimension = to_wgpu(spec.dimension),
            .baseMipLevel = spec.base_mip_level,
            .mipLevelCount = spec.mip_level_count,
            .baseArrayLayer = spec.base_array_layer,
            .arrayLayerCount = spec.array_layer_count,
            .aspect = to_wgpu(spec.aspect),
        };

        auto view = wgpuTextureCreateView(CAST(WGPUTexture, handle), &texture_view_descriptor);
        return TextureView{ view };
    }

    void Texture::release()
    {
        wgpuTextureRelease(CAST(WGPUTexture, handle));
        if (view != nullptr) {
            view.release();
        }
    }

    void BindGroup::release()
    {
        wgpuBindGroupRelease(as<WGPUBindGroup>());
    }

    void PipelineLayout::release()
    {
        wgpuPipelineLayoutRelease(as<WGPUPipelineLayout>());
    }

    void ShaderModule::release()
    {
        wgpuShaderModuleRelease(as<WGPUShaderModule>());
    }

    auto PrimitiveState::get_default() -> PrimitiveState
    {
        return PrimitiveState{
            .topology = PrimitiveTopology::TriangleList,
            .strip_index_format = None(),
            .front_face = FrontFace::Ccw,
            .cull = Face::None,
        };
    }

    auto DepthStencilState::get_default() -> DepthStencilState
    {
        return {
            .format = TextureFormat::Depth24Plus,
            .depth_write_enabled = true,
            .depth_compare = CompareFunction::Less,
            .stencil = {
                .front = {
                    .compare = CompareFunction::Always,
                    .fail_op = StencilOperation::Keep,
                    .depth_fail_op = StencilOperation::Keep,
                    .pass_op = StencilOperation::Keep },
                .back = { .compare = CompareFunction::Always, .fail_op = StencilOperation::Keep, .depth_fail_op = StencilOperation::Keep, .pass_op = StencilOperation::Keep },
                .read_mask = 0xFFFFFFFF,
                .write_mask = 0xFFFFFFFF,
            },
            .bias = {
                .constant = 0,
                .slope_scale = 0,
                .clamp = 0,
            }
        };
    }

    auto BlendState::get_default() -> BlendState
    {
        return {
            .color = {
                .src_factor = BlendFactor::SrcAlpha,
                .dst_factor = BlendFactor::OneMinusSrcAlpha,
                .operation = BlendOperation::Add },
            .alpha = { .src_factor = BlendFactor::Zero, .dst_factor = BlendFactor::One, .operation = BlendOperation::Add }
        };
    }

    auto MultiSampleState::get_default() -> MultiSampleState
    {
        return {
            .count = 1,
            .mask = ~0u,
            .alpha_to_coverage_enabled = false,
        };
    }

    auto RenderPipeline::bind_group_layout(u32 index) -> BindGroupLayout
    {
        auto layout = wgpuRenderPipelineGetBindGroupLayout(as<WGPURenderPipeline>(), index);
        return BindGroupLayout{ layout };
    }

    void RenderPipeline::release()
    {
        wgpuRenderPipelineRelease(as<WGPURenderPipeline>());
    }

    void RenderPassEncoder::set_viewport(Vector2 const& origin, Vector2 const& size, f32 min_depth, f32 max_depth) const
    {
        wgpuRenderPassEncoderSetViewport(as<WGPURenderPassEncoder>(), origin.x, origin.y, size.x, size.y, min_depth, max_depth);
    }

    void RenderPassEncoder::set_bind_group(BindGroup group, u32 index) const
    {
        wgpuRenderPassEncoderSetBindGroup(
            as<WGPURenderPassEncoder>(),
            index,
            CAST(WGPUBindGroup, group.handle),
            0, // TODO: Make configurable
            nullptr); // TODO: Make configurable
    }

    void RenderPassEncoder::set_vertex_buffer(u32 slot, BufferSlice const& slice) const
    {
        wgpuRenderPassEncoderSetVertexBuffer(as<WGPURenderPassEncoder>(), slot, slice.backing_buffer->as<WGPUBuffer>(), slice.start, slice.size);
    }

    void RenderPassEncoder::set_index_buffer(BufferSlice const& slice) const
    {
        wgpuRenderPassEncoderSetIndexBuffer(as<WGPURenderPassEncoder>(), slice.backing_buffer->as<WGPUBuffer>(), WGPUIndexFormat_Uint32, slice.start, slice.size);
    }

    void RenderPassEncoder::set_pipeline(RenderPipeline const& pipeline) const
    {
        wgpuRenderPassEncoderSetPipeline(as<WGPURenderPassEncoder>(), pipeline.as<WGPURenderPipeline>());
    }

    void RenderPassEncoder::draw(Range<u32> vertices, Range<u32> instances) const
    {
        wgpuRenderPassEncoderDraw(as<WGPURenderPassEncoder>(), vertices.count(), instances.count(), vertices.start, instances.start);
    }

    void RenderPassEncoder::draw_index(Range<u32> indices, Range<u32> instances) const
    {
        wgpuRenderPassEncoderDrawIndexed(
            as<WGPURenderPassEncoder>(),
            indices.count(),
            instances.stop,
            indices.start,
            0,
            instances.start);
    }

    void RenderPassEncoder::begin_pipeline_statistics_query(QuerySet const& set, u32 index) const
    {
        wgpuRenderPassEncoderBeginPipelineStatisticsQuery(as<WGPURenderPassEncoder>(), set.as<WGPUQuerySet>(), index);
    }

    void RenderPassEncoder::end_pipeline_statistics_query() const
    {
        wgpuRenderPassEncoderEndPipelineStatisticsQuery(as<WGPURenderPassEncoder>());
    }

    void RenderPassEncoder::end() const
    {
        wgpuRenderPassEncoderEnd(as<WGPURenderPassEncoder>());
    }

    void RenderPassEncoder::release()
    {
        wgpuRenderPassEncoderRelease(as<WGPURenderPassEncoder>());
    }

    auto CommandEncoder::begin_rendering(RenderPassSpec const& spec) const -> RenderPassEncoder
    {
        std::array<WGPURenderPassColorAttachment, 10> stack_attachments{};

        int i = 0;
        for (auto const& attachment : spec.color_attachments) {
            stack_attachments[i++] = {
                .nextInChain = nullptr,
                .view = CAST(WGPUTextureView, attachment.view.handle),
                .resolveTarget = nullptr,
                .loadOp = to_wgpu(attachment.load_op),
                .storeOp = to_wgpu(attachment.store_op),
                .clearValue = WGPUColor{
                    attachment.clear_color.r,
                    attachment.clear_color.g,
                    attachment.clear_color.b,
                    attachment.clear_color.a,
                },
            };
        }

        WGPURenderPassDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Render Pass"sv).data(),
            .colorAttachmentCount = spec.color_attachments.size(),
            .colorAttachments = stack_attachments.data(),
            .depthStencilAttachment = nullptr,
            .occlusionQuerySet = nullptr,
            .timestampWrites = nullptr,
        };

        WGPURenderPassTimestampWrites timestamp_writes{};
        if (spec.timestamp_writes) {
            timestamp_writes.querySet = spec.timestamp_writes->query_set.as<WGPUQuerySet>();
            if (spec.timestamp_writes->beginning_of_pass_write_index) {
                timestamp_writes.beginningOfPassWriteIndex = spec.timestamp_writes->beginning_of_pass_write_index.value();
            }
            if (spec.timestamp_writes->end_of_pass_write_index) {
                timestamp_writes.endOfPassWriteIndex = spec.timestamp_writes->end_of_pass_write_index.value();
            }
            desc.timestampWrites = &timestamp_writes;
        }

        if (auto depth = spec.depth_stencil_attachment) {
            WGPURenderPassDepthStencilAttachment d{
                .view = CAST(WGPUTextureView, depth->view.handle),
                .depthLoadOp = to_wgpu(depth->load_op),
                .depthStoreOp = to_wgpu(depth->store_op),
                .depthClearValue = depth->depth_clear,
                // .depthReadOnly = ,
                .stencilLoadOp = to_wgpu(LoadOp::Undefined),
                .stencilStoreOp = to_wgpu(StoreOp::Undefined),
                .stencilClearValue = 0,
                // .stencilReadOnly =
            };
            desc.depthStencilAttachment = &d;
            auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, handle), &desc);
            return RenderPassEncoder{ rp };
        }
        auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, handle), &desc);
        return RenderPassEncoder{ rp };
    }

    auto CommandEncoder::finish() -> CommandBuffer
    {
        WGPUCommandBufferDescriptor cmd_buffer_descriptor{
            .nextInChain = nullptr,
            .label = "Pepe Command Buffer"
        };
        auto cmd = wgpuCommandEncoderFinish(CAST(WGPUCommandEncoder, handle), &cmd_buffer_descriptor);
        return CommandBuffer{ cmd };
    }

    void CommandEncoder::copy_buffer_to_buffer(Buffer const& from, u64 from_offset, Buffer const& to, u64 to_offset, u64 size) const
    {
        wgpuCommandEncoderCopyBufferToBuffer(CAST(WGPUCommandEncoder, handle), from.as<WGPUBuffer>(), from_offset, to.as<WGPUBuffer>(), to_offset, size);
    }

    void CommandEncoder::copy_texture_to_texture(Texture const& from, Texture const& to, Vector2 const& size, u32 from_mip_level, u32 to_mip_level) const
    {
        WGPUImageCopyTexture source{
            .nextInChain = nullptr,
            .texture = from.as<WGPUTexture>(),
            .mipLevel = from_mip_level,
            .origin = { 0, 0, 0 },
            .aspect = WGPUTextureAspect_All,
        };
        WGPUImageCopyTexture dest{
            .nextInChain = nullptr,
            .texture = to.as<WGPUTexture>(),
            .mipLevel = to_mip_level,
            .origin = { 0, 0, 0 },
            .aspect = WGPUTextureAspect_All,
        };
        WGPUExtent3D copy_size{
            .width = CAST(u32, size.x),
            .height = CAST(u32, size.y),
            .depthOrArrayLayers = 1,
        };

        wgpuCommandEncoderCopyTextureToTexture(CAST(WGPUCommandEncoder, handle), &source, &dest, &copy_size);
    }

    void CommandEncoder::resolve_query_set(
        QuerySet const& set,
        Range<u32> query_range,
        Buffer const& destination,
        u64 destination_offset) const
    {
        wgpuCommandEncoderResolveQuerySet(
            CAST(WGPUCommandEncoder, handle),
            set.as<WGPUQuerySet>(),
            query_range.start, query_range.count(),
            destination.as<WGPUBuffer>(),
            destination_offset);
    }

    void CommandEncoder::release() const
    {
        wgpuCommandEncoderRelease(CAST(WGPUCommandEncoder, handle));
    }

    Limits Limits::default_()
    {
        return {};
    }

    Limits Limits::downlevel_defaults()
    {
        return {};
    }

    Device::Device(HandleT handle)
        : GPUHandle(handle)
    {
        queue = wgpuDeviceGetQueue(CAST(WGPUDevice, handle));
    }

    auto Device::create_buffer(BufferSpec const& spec) const -> Buffer
    {
        WGPUBufferDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Buffer"sv).data(),
            .usage = to_wgpu(spec.usage),
            .size = spec.size,
            .mappedAtCreation = spec.mapped
        };

        auto buffer = wgpuDeviceCreateBuffer(CAST(WGPUDevice, handle), &desc);
        return Buffer{ buffer, spec };
    }

    auto Device::create_texture(TextureSpec const& spec) const -> Texture
    {
        WGPUTextureDescriptor texture_descriptor{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Texture"sv).data(),
            .usage = to_wgpu(spec.usage),
            .dimension = to_wgpu(spec.dimension),
            .size = {
                .width = CAST(u32, spec.size.x),
                .height = CAST(u32, spec.size.y),
                .depthOrArrayLayers = CAST(u32, spec.size.z),
            },
            .format = to_wgpu(spec.format),
            .mipLevelCount = 1,
            .sampleCount = spec.sample_count,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
        };

        if (spec.generate_mip_maps) {
            texture_descriptor.mipLevelCount = CAST(u32, Math::floor_log2(Math::max(CAST(s32, spec.size.x), CAST(s32, spec.size.y)))) + 1;
        }

        auto texture_handle = wgpuDeviceCreateTexture(CAST(WGPUDevice, handle), &texture_descriptor);

        Texture texture{ texture_handle, spec };
        texture.mip_level_count = texture_descriptor.mipLevelCount;
        texture.initialize_view();
        return texture;
    }

    auto Device::create_sampler(SamplerSpec const& spec) const -> Sampler
    {
        WGPUSamplerDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Sampler"sv).data(),
            .addressModeU = to_wgpu(spec.address_mode_u),
            .addressModeV = to_wgpu(spec.address_mode_v),
            .addressModeW = to_wgpu(spec.address_mode_w),
            .magFilter = to_wgpu(spec.mag_filter),
            .minFilter = to_wgpu(spec.min_filter),
            .mipmapFilter = CAST(WGPUMipmapFilterMode, to_wgpu(spec.mip_map_filter)),
            .lodMinClamp = spec.lod_min_clamp,
            .lodMaxClamp = spec.lod_max_clamp,
            .compare = to_wgpu(spec.compare.value_or(CompareFunction::Undefined)),
            .maxAnisotropy = spec.anisotropy_clamp,
        };

        auto sampler = wgpuDeviceCreateSampler(as<WGPUDevice>(), &desc);
        return Sampler{ sampler };
    }

    auto Device::create_command_encoder(char const* label) const -> CommandEncoder
    {
        WGPUCommandEncoderDescriptor encoder_desc = {
            .nextInChain = nullptr,
            .label = label,
        };
        auto encoder = wgpuDeviceCreateCommandEncoder(CAST(WGPUDevice, handle), &encoder_desc);
        return CommandEncoder{ encoder };
    }

    auto Device::create_bind_group(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup
    {
        std::vector<WGPUBindGroupEntry> entries{};
        std::ranges::transform(spec.entries, std::back_inserter(entries), [](BindGroupEntry const& entry) {
            WGPUBindGroupEntry wgpu_entry{
                .nextInChain = nullptr,
                .binding = entry.binding,
                .buffer = nullptr,
                .offset = 0,
                .size = 0,
                .sampler = nullptr,
                .textureView = nullptr,
            };

            std::visit(overloaded{
                    [&](BufferBinding const& buffer_binding) {
                        wgpu_entry.buffer = CAST(WGPUBuffer, buffer_binding.buffer.handle);
                        wgpu_entry.offset = buffer_binding.offset;
                        wgpu_entry.size = buffer_binding.size;
                    },
                    [&](Sampler const& sampler) {
                        wgpu_entry.sampler = CAST(WGPUSampler, sampler.handle);
                    },
                    [&](TextureView const& view) {
                        wgpu_entry.textureView = CAST(WGPUTextureView, view.handle);
                    },
                    [](auto&&) {} },
                entry.resource);

            return wgpu_entry;
        });

        WGPUBindGroupDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Bind Group"sv).data(),
            .layout = CAST(WGPUBindGroupLayout, layout.handle),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bind_group = wgpuDeviceCreateBindGroup(CAST(WGPUDevice, handle), &desc);
        return BindGroup{ bind_group };
    }

    auto Device::create_bind_group_layout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout
    {
        std::vector<WGPUBindGroupLayoutEntry> entries{};
        std::ranges::transform(spec.entries, std::back_inserter(entries), [](BindGroupLayoutEntry const& entry) {
            WGPUBindGroupLayoutEntry wgpu_entry{
                .nextInChain = nullptr,
                .binding = entry.binding,
                .visibility = to_wgpu(entry.visibility),
            };

            std::visit(overloaded{
                    [&](BindingType::Buffer const& buffer) {
                        wgpu_entry.buffer = {
                            .nextInChain = nullptr,
                            .hasDynamicOffset = buffer.has_dynamic_offset,
                        };

                        wgpu_entry.buffer.type = std::visit(overloaded{
                                [&](BufferBindingType::Uniform const&) {
                                    return WGPUBufferBindingType_Uniform;
                                },
                                [&](BufferBindingType::Storage const& storage) {
                                    if (storage.read_only) {
                                        return WGPUBufferBindingType_ReadOnlyStorage;
                                    }
                                    return WGPUBufferBindingType_Storage;
                                },
                                [](auto&&) { return WGPUBufferBindingType_Undefined; } },
                            buffer.type);

                        if (buffer.min_binding_size) {
                            wgpu_entry.buffer.minBindingSize = *buffer.min_binding_size;
                        }
                    },
                    [&](BindingType::Sampler const& sampler) {
                        wgpu_entry.sampler.type = to_wgpu(sampler.type);
                    },
                    [&](BindingType::Texture const& texture) {
                        wgpu_entry.texture = {
                            .nextInChain = nullptr,
                            .viewDimension = to_wgpu(texture.view_dimension),
                            .multisampled = texture.multi_sampled,
                        };
                        wgpu_entry.texture.sampleType = std::visit(overloaded{
                                [](TextureSampleType::Float const& flt) {
                                    if (flt.filterable) {
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
                            texture.sample_type);
                    },
                    [&](BindingType::StorageTexture const& storage_texture) {
                        wgpu_entry.storageTexture = {
                            .nextInChain = nullptr,
                            .access = to_wgpu(storage_texture.access),
                            .format = to_wgpu(storage_texture.format),
                            .viewDimension = to_wgpu(storage_texture.view_dimension),
                        };
                    },
                    [](BindingType::AccelerationStructure&&) {
                        PANIC("TODO!");
                    },
                    [](auto&&) {},
                },
                entry.type);

            return wgpu_entry;
        });

        WGPUBindGroupLayoutDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Bind Group"sv).data(),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bg_layout = wgpuDeviceCreateBindGroupLayout(CAST(WGPUDevice, handle), &desc);
        return BindGroupLayout{ bg_layout };
    }

    auto Device::create_query_set(QuerySetSpec const& spec) const -> QuerySet
    {
        // This is declared outside the visit lambdas because it will be reffered
        // to, by a pointer from the WGPUQuerySetDescriptorExtras struct.
        std::vector<WGPUPipelineStatisticName> pipeline_statistic_names{};
        pipeline_statistic_names.reserve(5);

        WGPUQuerySetDescriptorExtras extras{
            .chain = {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_QuerySetDescriptorExtras)
            },
        };

        WGPUQuerySetDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("QuerySet").data.ptr,
            .count = spec.count,
        };

        std::visit(overloaded{
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
        }, spec.type);

        auto set = wgpuDeviceCreateQuerySet(CAST(WGPUDevice, handle), &desc);
        return QuerySet(set, spec);
    }

    auto Device::create_shader_module(ShaderModuleSpec const& spec) const -> ShaderModule
    {
        auto desc = std::visit(overloaded{
                [](WGSLShader const& wgsl) {
                    WGPUShaderModuleWGSLDescriptor wgsl_descriptor{
                        .chain = {
                            .next = nullptr,
                            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
                        },
                        .code = wgsl.source.data(),
                    };
                    WGPUShaderModuleDescriptor desc{
                        .nextInChain = &wgsl_descriptor.chain,
                        .label = "Shader",
                        .hintCount = 0,
                        .hints = nullptr,
                    };
                    return desc;
                },
                [](SPIRVShader const& spirv) {
                    WGPUShaderModuleSPIRVDescriptor spirv_descriptor{
                        .chain = {
                            .next = nullptr,
                            .sType = WGPUSType_ShaderModuleSPIRVDescriptor,
                        },
                        .codeSize = CAST(u32, spirv.binary.size_bytes()),
                        .code = spirv.binary.data(),
                    };
                    WGPUShaderModuleDescriptor desc{
                        .nextInChain = &spirv_descriptor.chain,
                        .label = "Shader",
                        .hintCount = 0,
                        .hints = nullptr,
                    };

                    return desc;
                },
                [](auto&&) {},
            },
            spec.type);

        auto module = wgpuDeviceCreateShaderModule(CAST(WGPUDevice, handle), &desc);

        return ShaderModule{ module, spec };
    }

    auto Device::create_pipeline_layout(PipelineLayoutSpec const& spec) const -> PipelineLayout
    {
        std::vector<WGPUBindGroupLayout> layouts{};
        std::ranges::transform(spec.bind_group_layouts, std::back_inserter(layouts), [](BindGroupLayout const& bind_group_layout) {
            return CAST(WGPUBindGroupLayout, bind_group_layout.handle);
        });

        WGPUPipelineLayoutDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Pipeline Layout"sv).data(),
            .bindGroupLayoutCount = CAST(u32, layouts.size()),
            .bindGroupLayouts = layouts.data(),
        };

        auto layout = wgpuDeviceCreatePipelineLayout(CAST(WGPUDevice, handle), &desc);
        return PipelineLayout{ layout };
    }

    auto Device::create_render_pipeline(ShaderModule const& vert_module, ShaderModule const& frag_module, RenderPipelineSpec const& spec) const -> RenderPipeline
    {
        std::vector<WGPUVertexBufferLayout> vertex_buffer_layouts{};
        std::vector<std::vector<WGPUVertexAttribute>> attributes{};
        attributes.resize(spec.vertex.attribute_layouts.size());

        size_t attribute_index = 0;
        std::ranges::transform(spec.vertex.attribute_layouts, std::back_inserter(vertex_buffer_layouts), [&](VertexBufferLayout const& layout) {
            std::ranges::transform(layout.attributes, std::back_inserter(attributes[attribute_index]), [](VertexAttribute const& attribute) {
                return WGPUVertexAttribute{
                    .format = to_wgpu(attribute.type),
                    .offset = attribute.offset,
                    .shaderLocation = attribute.shader_location,
                };
            });

            WGPUVertexBufferLayout wgpu_layout{
                .arrayStride = layout.array_stride,
                .stepMode = to_wgpu(layout.step_mode),
                .attributeCount = CAST(u32, attributes[attribute_index].size()),
                .attributes = attributes[attribute_index].data(),
            };

            attribute_index++;
            return wgpu_layout;
        });

        WGPUVertexState vertex{
            .nextInChain = nullptr,
            .module = vert_module.as<WGPUShaderModule>(),
            .entryPoint = vert_module.spec.vertex_entry_point.data(),
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = CAST(u32, vertex_buffer_layouts.size()),
            .buffers = vertex_buffer_layouts.data(),
        };

        WGPUPrimitiveState primitive{
            .nextInChain = nullptr,
            .topology = to_wgpu(spec.primitive.topology),
            .stripIndexFormat = to_wgpu(spec.primitive.strip_index_format.value_or(IndexFormat::Undefined)),
            .frontFace = to_wgpu(spec.primitive.front_face),
            .cullMode = to_wgpu(spec.primitive.cull),
        };

        if (spec.primitive.strip_index_format) {
            primitive.stripIndexFormat = to_wgpu(*spec.primitive.strip_index_format);
        }

        WGPUMultisampleState multisample{
            .nextInChain = nullptr,
            .count = spec.multi_sample.count,
            .mask = spec.multi_sample.mask,
            .alphaToCoverageEnabled = spec.multi_sample.alpha_to_coverage_enabled,
        };

        std::vector<WGPUColorTargetState> color_targets{};
        // We need to save these here because the color
        // target state gets a pointer to the blend state.
        std::vector<WGPUBlendState> blends{};
        WGPUFragmentState fragment{};

        if (spec.fragment.has_value()) {
            fragment.module = frag_module.as<WGPUShaderModule>();
            fragment.entryPoint = frag_module.spec.fragment_entry_point.data();
            fragment.constantCount = 0;
            fragment.constants = nullptr;
            // Resize to a max of spec.Fragment.Targets.size() to prevent reallocations
            // and dangling pointers.
            blends.reserve(spec.fragment->targets.size());

            std::ranges::transform(spec.fragment->targets, std::back_inserter(color_targets), [&blends](ColorTargetState const& state) {
                WGPUColorTargetState wgpu_state{
                    .nextInChain = nullptr,
                    .format = to_wgpu(state.format),
                    .blend = nullptr,
                    .writeMask = to_wgpu(state.write_mask),
                };

                if (auto b = state.blend) {
                    auto& blend = blends.emplace_back();
                    blend = {
                        .color = {
                            .operation = to_wgpu(b->color.operation),
                            .srcFactor = to_wgpu(b->color.src_factor),
                            .dstFactor = to_wgpu(b->color.dst_factor)
                        },
                        .alpha = {
                            .operation = to_wgpu(b->alpha.operation),
                            .srcFactor = to_wgpu(b->alpha.src_factor),
                            .dstFactor = to_wgpu(b->alpha.dst_factor)
                        }
                    };

                    wgpu_state.blend = &blend;
                }

                return wgpu_state;
            });

            fragment.targetCount = CAST(u32, color_targets.size());
            fragment.targets = color_targets.data();
        }

        WGPURenderPipelineDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Render Pipeline"sv).data(),
            .vertex = vertex,
            .primitive = primitive,
            .depthStencil = nullptr,
            .multisample = multisample,
            .fragment = spec.fragment ? &fragment : nullptr,
        };
        if (spec.layout.has_value()) {
            desc.layout = spec.layout->as<WGPUPipelineLayout>();
        }

        WGPUDepthStencilState depth_stencil;
        if (spec.depth_stencil) {
            depth_stencil = {
                .nextInChain = nullptr,
                .format = to_wgpu(spec.depth_stencil->format),
                .depthWriteEnabled = spec.depth_stencil->depth_write_enabled,
                .depthCompare = to_wgpu(spec.depth_stencil->depth_compare),
                .stencilFront = {
                    .compare = to_wgpu(spec.depth_stencil->stencil.front.compare),
                    .failOp = to_wgpu(spec.depth_stencil->stencil.front.fail_op),
                    .depthFailOp = to_wgpu(spec.depth_stencil->stencil.front.depth_fail_op),
                    .passOp = to_wgpu(spec.depth_stencil->stencil.front.pass_op),
                },
                .stencilBack = {
                    .compare = to_wgpu(spec.depth_stencil->stencil.back.compare),
                    .failOp = to_wgpu(spec.depth_stencil->stencil.back.fail_op),
                    .depthFailOp = to_wgpu(spec.depth_stencil->stencil.back.depth_fail_op),
                    .passOp = to_wgpu(spec.depth_stencil->stencil.back.pass_op),
                },
                .stencilReadMask = spec.depth_stencil->stencil.read_mask,
                .stencilWriteMask = spec.depth_stencil->stencil.write_mask,
                .depthBias = spec.depth_stencil->bias.constant,
                .depthBiasSlopeScale = spec.depth_stencil->bias.slope_scale,
                .depthBiasClamp = spec.depth_stencil->bias.clamp
            };
            desc.depthStencil = &depth_stencil;
        }

        auto pipeline = wgpuDeviceCreateRenderPipeline(CAST(WGPUDevice, handle), &desc);
        return RenderPipeline{ pipeline };
    }

    std::mutex QueueMutex{};

    void Device::submit_command_buffer(CommandBuffer cmd) const
    {
        ZoneScoped;
        std::lock_guard lock(QueueMutex);

        WGPUCommandBuffer cmds[] = {
            CAST(WGPUCommandBuffer, cmd.handle),
        };
        wgpuQueueSubmit(CAST(WGPUQueue, queue), 1, cmds);
    }

    void Device::write_texture(
        Texture const& texture,
        void const* data,
        size_t data_size,
        Vector2 const& origin,
        Vector2 const& size,
        u32 bytes_per_pixel,
        u32 mip_level) const
    {
        WGPUImageCopyTexture copy_texture{
            .nextInChain = nullptr,
            .texture = CAST(WGPUTexture, texture.handle),
            .mipLevel = mip_level,
            .origin = {
                .x = CAST(u32, origin.x),
                .y = CAST(u32, origin.y),
                .z = 0,
            },
            .aspect = WGPUTextureAspect_All,
        };

        WGPUTextureDataLayout layout{
            .nextInChain = nullptr,
            .offset = 0,
            .bytesPerRow = CAST(u32, size.x * bytes_per_pixel),
            .rowsPerImage = CAST(u32, size.y),
        };

        WGPUExtent3D texture_size{
            .width = CAST(u32, texture.spec.size.x),
            .height = CAST(u32, texture.spec.size.y),
            .depthOrArrayLayers = 1,
        };
        wgpuQueueWriteTexture(CAST(WGPUQueue, queue), &copy_texture, data, data_size, &layout, &texture_size);
        wgpuDevicePoll(as<WGPUDevice>(), true, nullptr);
    }

    void Device::release()
    {
        wgpuDeviceRelease(as<WGPUDevice>());
    }

    void Device::write_buffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const
    {
        wgpuQueueWriteBuffer(CAST(WGPUQueue, queue), CAST(WGPUBuffer, buffer.handle), offset, data, size);
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

    auto Adapter::device(DeviceSpec const& spec) -> Device
    {
        std::vector<WGPUFeatureName> features{};
        for (auto const& feature : spec.required_features) {
            features.push_back(to_wgpu(feature));
        }

        WGPUDeviceDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.label.value_or("Device").data.ptr,
            .requiredFeatureCount = features.size(),
            .requiredFeatures = features.data(),
            .defaultQueue = {
                .nextInChain = nullptr,
                .label = "Default Queue"
            },
            .deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
                PANIC("!DEVICE LOST!: \n\tREASON: {}\n\tMESSAGE: {}", magic_enum::enum_name(reason), message);
            },
            .deviceLostUserdata = this,
            .uncapturedErrorCallbackInfo = {
                .callback = [](WGPUErrorType type, char const* message, void* userdata) {
                    LOG_ERRORF("!DEVICE ERROR!\n\tTYPE: {}\n\tMESSAGE: {}", magic_enum::enum_name(type), message);
                },
                .userdata = this,
            },
        };

        auto device = request_device_sync(CAST(WGPUAdapter, handle), &desc);

        return Device{ device };
    }

    void Adapter::release() const
    {
        wgpuAdapterRelease(CAST(WGPUAdapter, handle));
    }

    void Surface::release() const
    {
        wgpuSurfaceRelease(CAST(WGPUSurface, handle));
    }

    void Surface::configure(Device const& device, Adapter adapter, Config const& config)
    {
        WGPUSurfaceCapabilities caps{};
        wgpuSurfaceGetCapabilities(CAST(WGPUSurface, handle), CAST(WGPUAdapter, adapter.handle), &caps);
        VERIFY(caps.formatCount >= 1, "Surface without formats?!!!");
        LOG_DEBUGF("Configuring surface with format: {}", magic_enum::enum_name(caps.formats[0]));

        WGPUSurfaceConfigurationExtras extras{
            .chain = WGPUChainedStruct{
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_SurfaceConfigurationExtras),
            },
            .desiredMaximumFrameLatency = 2
        };
        WGPUSurfaceConfiguration conf{
            .nextInChain = &extras.chain,
            .device = CAST(WGPUDevice, device.handle),
            .format = caps.formats[0], // TODO: Check this properly
            .usage = WGPUTextureUsage_RenderAttachment,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
            .alphaMode = WGPUCompositeAlphaMode_Auto,
            .width = CAST(u32, config.size.x),
            .height = CAST(u32, config.size.y),
            .presentMode = to_wgpu(config.present_mode),
        };
        wgpuSurfaceConfigure(CAST(WGPUSurface, handle), &conf);

        format = from_wgpu(caps.formats[0]);
    }

    auto Surface::get_next_view() const -> Result<TextureView, Error>
    {
        WGPUSurfaceTexture texture;
        {
            ZoneScopedN("GetCurrentTexture");
            wgpuSurfaceGetCurrentTexture(CAST(WGPUSurface, handle), &texture);
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
        return TextureView{ target_view };
    }

    void Surface::present() const
    {
        ZoneScoped;
        wgpuSurfacePresent(CAST(WGPUSurface, handle));
    }

    Instance Instance::create(InstanceSpec const& spec)
    {
        Instance instance;
        WGPUInstanceExtras instance_extras{
            .chain = {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_InstanceExtras),
            },
            .backends = to_wgpu(spec.backend),
        };

        WGPUInstanceDescriptor desc{
            .nextInChain = &instance_extras.chain,
        };
        instance.handle = wgpuCreateInstance(&desc);
        return instance;
    }

    void Instance::release() const
    {
        wgpuInstanceRelease(CAST(WGPUInstance, handle));
    }

    auto Instance::surface(Window const* window) const -> Surface
    {
        auto glfw_window = CAST(GLFWwindow*, window->native_handle());
        auto surface = glfwGetWGPUSurface(CAST(WGPUInstance, handle), glfw_window);
        return Surface{ surface };
    }

    GlobalReport Instance::generate_global_report() const
    {
        WGPUGlobalReport report{};
        wgpuGenerateReport(CAST(WGPUInstance, handle), &report);

        GlobalReport my_report{
            .adapters = {
                .num_allocated = report.vulkan.adapters.numAllocated,
                .num_kept_from_user = report.vulkan.adapters.numKeptFromUser,
                .num_released_from_user = report.vulkan.adapters.numReleasedFromUser,
                .num_error = report.vulkan.adapters.numError,
                .element_size = report.vulkan.adapters.elementSize
            },
            .devices = {
                .num_allocated = report.vulkan.devices.numAllocated,
                .num_kept_from_user = report.vulkan.devices.numKeptFromUser,
                .num_released_from_user = report.vulkan.devices.numReleasedFromUser,
                .num_error = report.vulkan.devices.numError,
                .element_size = report.vulkan.devices.elementSize
            },
            .queues = {
                .num_allocated = report.vulkan.queues.numAllocated,
                .num_kept_from_user = report.vulkan.queues.numKeptFromUser,
                .num_released_from_user = report.vulkan.queues.numReleasedFromUser,
                .num_error = report.vulkan.queues.numError,
                .element_size = report.vulkan.queues.elementSize
            },
            .pipeline_layouts = {
                .num_allocated = report.vulkan.pipelineLayouts.numAllocated,
                .num_kept_from_user = report.vulkan.pipelineLayouts.numKeptFromUser,
                .num_released_from_user = report.vulkan.pipelineLayouts.numReleasedFromUser,
                .num_error = report.vulkan.pipelineLayouts.numError,
                .element_size = report.vulkan.pipelineLayouts.elementSize
            },
            .shader_modules = {
                .num_allocated = report.vulkan.shaderModules.numAllocated,
                .num_kept_from_user = report.vulkan.shaderModules.numKeptFromUser,
                .num_released_from_user = report.vulkan.shaderModules.numReleasedFromUser,
                .num_error = report.vulkan.shaderModules.numError,
                .element_size = report.vulkan.shaderModules.elementSize
            },
            .bind_group_layouts = {
                .num_allocated = report.vulkan.bindGroupLayouts.numAllocated,
                .num_kept_from_user = report.vulkan.bindGroupLayouts.numKeptFromUser,
                .num_released_from_user = report.vulkan.bindGroupLayouts.numReleasedFromUser,
                .num_error = report.vulkan.bindGroupLayouts.numError,
                .element_size = report.vulkan.bindGroupLayouts.elementSize
            },
            .bind_groups = {
                .num_allocated = report.vulkan.bindGroups.numAllocated,
                .num_kept_from_user = report.vulkan.bindGroups.numKeptFromUser,
                .num_released_from_user = report.vulkan.bindGroups.numReleasedFromUser,
                .num_error = report.vulkan.bindGroups.numError,
                .element_size = report.vulkan.bindGroups.elementSize
            },
            .command_buffers = {
                .num_allocated = report.vulkan.commandBuffers.numAllocated,
                .num_kept_from_user = report.vulkan.commandBuffers.numKeptFromUser,
                .num_released_from_user = report.vulkan.commandBuffers.numReleasedFromUser,
                .num_error = report.vulkan.commandBuffers.numError,
                .element_size = report.vulkan.commandBuffers.elementSize
            },
            .render_bundles = {
                .num_allocated = report.vulkan.renderBundles.numAllocated,
                .num_kept_from_user = report.vulkan.renderBundles.numKeptFromUser,
                .num_released_from_user = report.vulkan.renderBundles.numReleasedFromUser,
                .num_error = report.vulkan.renderBundles.numError,
                .element_size = report.vulkan.renderBundles.elementSize
            },
            .render_pipelines = {
                .num_allocated = report.vulkan.renderPipelines.numAllocated,
                .num_kept_from_user = report.vulkan.renderPipelines.numKeptFromUser,
                .num_released_from_user = report.vulkan.renderPipelines.numReleasedFromUser,
                .num_error = report.vulkan.renderPipelines.numError,
                .element_size = report.vulkan.renderPipelines.elementSize
            },
            .compute_pipelines = {
                .num_allocated = report.vulkan.computePipelines.numAllocated,
                .num_kept_from_user = report.vulkan.computePipelines.numKeptFromUser,
                .num_released_from_user = report.vulkan.computePipelines.numReleasedFromUser,
                .num_error = report.vulkan.computePipelines.numError,
                .element_size = report.vulkan.computePipelines.elementSize
            },
            .query_sets = {
                .num_allocated = report.vulkan.querySets.numAllocated,
                .num_kept_from_user = report.vulkan.querySets.numKeptFromUser,
                .num_released_from_user = report.vulkan.querySets.numReleasedFromUser,
                .num_error = report.vulkan.querySets.numError,
                .element_size = report.vulkan.querySets.elementSize
            },
            .buffers = {
                .num_allocated = report.vulkan.buffers.numAllocated,
                .num_kept_from_user = report.vulkan.buffers.numKeptFromUser,
                .num_released_from_user = report.vulkan.buffers.numReleasedFromUser,
                .num_error = report.vulkan.buffers.numError,
                .element_size = report.vulkan.buffers.elementSize
            },
            .textures = {
                .num_allocated = report.vulkan.textures.numAllocated,
                .num_kept_from_user = report.vulkan.textures.numKeptFromUser,
                .num_released_from_user = report.vulkan.textures.numReleasedFromUser,
                .num_error = report.vulkan.textures.numError,
                .element_size = report.vulkan.textures.elementSize
            },
            .texture_views = {
                .num_allocated = report.vulkan.textureViews.numAllocated,
                .num_kept_from_user = report.vulkan.textureViews.numKeptFromUser,
                .num_released_from_user = report.vulkan.textureViews.numReleasedFromUser,
                .num_error = report.vulkan.textureViews.numError,
                .element_size = report.vulkan.textureViews.elementSize
            },
            .samplers = {
                .num_allocated = report.vulkan.samplers.numAllocated,
                .num_kept_from_user = report.vulkan.samplers.numKeptFromUser,
                .num_released_from_user = report.vulkan.samplers.numReleasedFromUser,
                .num_error = report.vulkan.samplers.numError,
                .element_size = report.vulkan.samplers.elementSize
            }
        };
        return my_report;
    }

    auto Instance::adapter(Surface surface, AdapterOptions const& opt) const -> Adapter
    {
        WGPURequestAdapterOptions options{
            .nextInChain = nullptr,
            .compatibleSurface = CAST(WGPUSurface, surface.handle),
            .powerPreference = to_wgpu(opt.power_preference),
        };
        auto adapter = request_adapter_sync(CAST(WGPUInstance, handle), &options);
        return Adapter{ adapter };
    }

    void CommandBuffer::release() const
    {
        ZoneScoped;
        wgpuCommandBufferRelease(CAST(WGPUCommandBuffer, handle));
    }
}
