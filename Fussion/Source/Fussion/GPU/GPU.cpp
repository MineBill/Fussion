#include "FussionPCH.h"
#include "GPU.h"
#include "EnumConversions.h"

#include "glfw3webgpu.h"
#include <webgpu/wgpu.h>
#include <GLFW/glfw3.h>

#include <magic_enum/magic_enum.hpp>

namespace Fussion::GPU {
    template<class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // Utility function to retrieve the adapter without callbacks.
    WGPUAdapter RequestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const* options)
    {
        struct UserData {
            WGPUAdapter Adapter;
            bool RequestEnded;
        } user_data;
        wgpuInstanceRequestAdapter(instance, options,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message,
            void* pUserData) {
                auto user_data = static_cast<UserData*>(pUserData);

                if (status == WGPURequestAdapterStatus_Success) {
                    user_data->Adapter = adapter;
                } else {
                    LOG_ERRORF("Could not get adapter: {}", message);
                }
                user_data->RequestEnded = true;
            }, &user_data);

        return user_data.Adapter;
    }

    // Utility function to retrieve the device without callbacks.
    WGPUDevice RequestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor)
    {
        struct UserData {
            WGPUDevice Device;
            bool RequestEnded;
        } user_data;

        wgpuAdapterRequestDevice(adapter, descriptor,
            [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message,
            void* pUserData) {
                auto user_data = static_cast<UserData*>(pUserData);

                if (status == WGPURequestDeviceStatus_Success) {
                    user_data->Device = device;
                } else {
                    LOG_ERRORF("Could not get device: {}", message);
                }
                user_data->RequestEnded = true;
            }, &user_data);

        return user_data.Device;
    }

    auto Buffer::GetSize() const -> u64
    {
        return wgpuBufferGetSize(CAST(WGPUBuffer, Handle));
    }

    auto Buffer::GetSlice(u32 start, u32 size) -> BufferSlice
    {
        return BufferSlice(*this, start, size);
    }

    void Buffer::Release()
    {
        wgpuBufferRelease(CAST(WGPUBuffer, Handle));
    }

    BufferSlice::BufferSlice(Buffer const& buffer, u32 start, u32 size): BackingBuffer(buffer), Start(start), Size(size) {}

    BufferSlice::BufferSlice(Buffer const& buffer): BackingBuffer(buffer), Start(0), Size(buffer.GetSize()) {}

    void TextureView::Release()
    {
        wgpuTextureViewRelease(CAST(WGPUTextureView, Handle));
    }

    void Texture::InitializeView()
    {
        View = CreateView({
            .Label = "View",
            .Usage = Spec.Usage,
            .Dimension = TextureViewDimension::D2, // TODO: Make configurable
            .Format = Spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = Spec.MipLevelCount,
            .BaseArrayLayer = 0, // TODO: Make configurable
            .ArrayLayerCount = 1, // TODO: Make configurable
            .Aspect = TextureAspect::All // TODO: Make configurable
        });
    }

    TextureView Texture::CreateView(TextureViewSpec const& spec) const
    {
        WGPUTextureViewDescriptor texture_view_descriptor{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("View"),
            .format = ToWgpu(spec.Format),
            .dimension = ToWgpu(spec.Dimension),
            .baseMipLevel = spec.BaseMipLevel,
            .mipLevelCount = spec.MipLevelCount,
            .baseArrayLayer = spec.BaseArrayLayer,
            .arrayLayerCount = spec.ArrayLayerCount,
            .aspect = ToWgpu(spec.Aspect),
        };

        auto view = wgpuTextureCreateView(CAST(WGPUTexture, Handle), &texture_view_descriptor);
        return TextureView{ view };
    }

    void Texture::Release()
    {
        wgpuTextureRelease(CAST(WGPUTexture, Handle));
    }

    void PipelineLayout::Release()
    {
        wgpuPipelineLayoutRelease(As<WGPUPipelineLayout>());
    }

    void ShaderModule::Release()
    {
        wgpuShaderModuleRelease(As<WGPUShaderModule>());
    }

    void RenderPipeline::Release()
    {
        wgpuRenderPipelineRelease(As<WGPURenderPipeline>());
    }

    void RenderPassEncoder::SetViewport(Vector2 const& origin, Vector2 const& size, f32 min_depth, f32 max_depth) const
    {
        wgpuRenderPassEncoderSetViewport(As<WGPURenderPassEncoder>(), origin.X, origin.Y, size.X, size.Y, min_depth, max_depth);
    }

    void RenderPassEncoder::SetBindGroup(BindGroup group, u32 index) const
    {
        wgpuRenderPassEncoderSetBindGroup(
            As<WGPURenderPassEncoder>(),
            index,
            CAST(WGPUBindGroup, group.Handle),
            0, // TODO: Make configurable
            nullptr); // TODO: Make configurable
    }

    void RenderPassEncoder::SetVertexBuffer(u32 slot, BufferSlice const& slice) const
    {
        wgpuRenderPassEncoderSetVertexBuffer(As<WGPURenderPassEncoder>(), slot, slice.BackingBuffer.As<WGPUBuffer>(), slice.Start, slice.Size);
    }

    void RenderPassEncoder::SetPipeline(RenderPipeline const& pipeline) const
    {
        wgpuRenderPassEncoderSetPipeline(As<WGPURenderPassEncoder>(), pipeline.As<WGPURenderPipeline>());
    }

    void RenderPassEncoder::Draw(Range<u32> vertices, Range<u32> instances) const
    {
        wgpuRenderPassEncoderDraw(As<WGPURenderPassEncoder>(), vertices.Count(), instances.Count(), vertices.Start, instances.Start);
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
        std::array<WGPURenderPassColorAttachment, 10> stack_attachments{};

        int i = 0;
        for (auto const& attachment : spec.ColorAttachments) {
            stack_attachments[i++] = {
                .nextInChain = nullptr,
                .view = CAST(WGPUTextureView, attachment.View.Handle),
                .resolveTarget = nullptr,
                .loadOp = ToWgpu(attachment.LoadOp),
                .storeOp = ToWgpu(attachment.StoreOp),
                .clearValue = WGPUColor{
                    attachment.ClearColor.R,
                    attachment.ClearColor.G,
                    attachment.ClearColor.B,
                    attachment.ClearColor.A,
                },
            };
        }
        WGPURenderPassDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Render Pass"),
            .colorAttachmentCount = spec.ColorAttachments.size(),
            .colorAttachments = stack_attachments.data(),
            .depthStencilAttachment = nullptr,
            .occlusionQuerySet = nullptr,
            .timestampWrites = nullptr,
        };

        if (auto depth = spec.DepthStencilAttachment) {
            WGPURenderPassDepthStencilAttachment d{
                .view = CAST(WGPUTextureView, depth->View.Handle),
                .depthLoadOp = ToWgpu(depth->LoadOp),
                .depthStoreOp = ToWgpu(depth->StoreOp),
                .depthClearValue = depth->DepthClear,
                // .depthReadOnly = ,
                .stencilLoadOp = ToWgpu(LoadOp::Undefined),
                .stencilStoreOp = ToWgpu(StoreOp::Undefined),
                .stencilClearValue = 0,
                // .stencilReadOnly =
            };
            desc.depthStencilAttachment = &d;
            auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, Handle), &desc);
            return RenderPassEncoder{ rp };
        }
        auto rp = wgpuCommandEncoderBeginRenderPass(CAST(WGPUCommandEncoder, Handle), &desc);
        return RenderPassEncoder{ rp };
    }

    auto CommandEncoder::Finish() -> CommandBuffer
    {
        WGPUCommandBufferDescriptor cmd_buffer_descriptor{
            .nextInChain = nullptr,
            .label = "Pepe Command Buffer"
        };
        auto cmd = wgpuCommandEncoderFinish(CAST(WGPUCommandEncoder, Handle), &cmd_buffer_descriptor);
        return CommandBuffer{ cmd };
    }

    void CommandEncoder::Release() const
    {
        wgpuCommandEncoderRelease(CAST(WGPUCommandEncoder, Handle));
    }

    Device::Device(HandleT handle): Handle(handle)
    {
        Queue = wgpuDeviceGetQueue(CAST(WGPUDevice, Handle));
    }

    void Device::Release() const
    {
        wgpuDeviceRelease(CAST(WGPUDevice, Handle));
    }

    auto Device::CreateBuffer(BufferSpec const& spec) const -> Buffer
    {
        WGPUBufferDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label,
            .usage = ToWgpu(spec.Usage),
            .size = spec.Size,
            .mappedAtCreation = spec.Mapped
        };

        auto buffer = wgpuDeviceCreateBuffer(CAST(WGPUDevice, Handle), &desc);
        return Buffer{ buffer, spec };
    }

    auto Device::CreateTexture(TextureSpec const& spec) const -> Texture
    {
        WGPUTextureDescriptor texture_descriptor{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Texture"),
            .usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding,
            .dimension = ToWgpu(spec.Dimension),
            .size = {
                .width = CAST(u32, spec.Size.X),
                .height = CAST(u32, spec.Size.Y),
                .depthOrArrayLayers = CAST(u32, spec.Size.Z),
            },
            .format = ToWgpu(spec.Format),
            .mipLevelCount = spec.MipLevelCount,
            .sampleCount = spec.SampleCount,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
        };

        auto texture_handle = wgpuDeviceCreateTexture(CAST(WGPUDevice, Handle), &texture_descriptor);

        Texture texture{ texture_handle, spec };
        texture.InitializeView();
        return texture;
    }

    auto Device::CreateCommandEncoder(const char* label) const -> CommandEncoder
    {
        WGPUCommandEncoderDescriptor encoder_desc = {
            .nextInChain = nullptr,
            .label = label,
        };
        auto encoder = wgpuDeviceCreateCommandEncoder(CAST(WGPUDevice, Handle), &encoder_desc);
        return CommandEncoder{ encoder };
    }

    auto Device::CreateBindGroup(BindGroupLayout layout, BindGroupSpec const& spec) const -> BindGroup
    {
        std::vector<WGPUBindGroupEntry> entries{};
        std::ranges::transform(spec.Entries, std::back_inserter(entries), [](BindGroupEntry const& entry) {
            WGPUBindGroupEntry wgpu_entry{
                .nextInChain = nullptr,
                .binding = entry.Binding,
                .buffer = nullptr,
                .offset = 0,
                .size = 0,
                .sampler = nullptr,
                .textureView = nullptr,
            };

            std::visit(overloaded{
                [&](BufferBinding const& buffer_binding) {
                    wgpu_entry.buffer = CAST(WGPUBuffer, buffer_binding.Buffer.Handle);
                    wgpu_entry.offset = buffer_binding.Offset;
                    wgpu_entry.size = buffer_binding.Size;
                },
                [&](Sampler const& sampler) {
                    wgpu_entry.sampler = CAST(WGPUSampler, sampler.Handle);
                },
                [&](TextureView const& view) {
                    wgpu_entry.textureView = CAST(WGPUTextureView, view.Handle);
                },
                [](auto&&) {}
            }, entry.Resource);

            return wgpu_entry;
        });

        WGPUBindGroupDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Bind Group"),
            .layout = CAST(WGPUBindGroupLayout, layout.Handle),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bind_group = wgpuDeviceCreateBindGroup(CAST(WGPUDevice, Handle), &desc);
        return BindGroup{ bind_group };
    }

    auto Device::CreateBindGroupLayout(BindGroupLayoutSpec const& spec) const -> BindGroupLayout
    {
        std::vector<WGPUBindGroupLayoutEntry> entries{};
        std::ranges::transform(spec.Entries, std::back_inserter(entries), [](BindGroupLayoutEntry const& entry) {
            WGPUBindGroupLayoutEntry wgpu_entry{
                .nextInChain = nullptr,
                .binding = entry.Binding,
                .visibility = ToWgpu(entry.Visibility),
            };

            std::visit(overloaded{
                [&](BindingType::Buffer const& buffer) {
                    wgpu_entry.buffer = {
                        .nextInChain = nullptr,
                        .hasDynamicOffset = buffer.HasDynamicOffset,
                    };

                    wgpu_entry.buffer.type = std::visit(overloaded{
                        [&](BufferBindingType::Uniform const&) {
                            return WGPUBufferBindingType_Uniform;
                        },
                        [&](BufferBindingType::Storage const& storage) {
                            if (storage.ReadOnly) {
                                return WGPUBufferBindingType_ReadOnlyStorage;
                            }
                            return WGPUBufferBindingType_Storage;
                        },
                        [](auto&&) { return WGPUBufferBindingType_Undefined; }
                    }, buffer.Type);

                    if (buffer.MinBindingSize) {
                        wgpu_entry.buffer.minBindingSize = *buffer.MinBindingSize;
                    }
                },
                [&](BindingType::Sampler const& sampler) {
                    wgpu_entry.sampler.type = ToWgpu(sampler.Type);
                },
                [&](BindingType::Texture const& texture) {
                    wgpu_entry.texture = {
                        .nextInChain = nullptr,
                        .viewDimension = ToWgpu(texture.ViewDimension),
                        .multisampled = texture.MultiSampled,
                    };
                    wgpu_entry.texture.sampleType = std::visit(overloaded{
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
                        [](auto&&) { return WGPUTextureSampleType_Undefined; }
                    }, texture.SampleType);
                },
                [&](BindingType::StorageTexture const& storage_texture) {
                    wgpu_entry.storageTexture = {
                        .nextInChain = nullptr,
                        .access = ToWgpu(storage_texture.Access),
                        .format = ToWgpu(storage_texture.Format),
                        .viewDimension = ToWgpu(storage_texture.ViewDimension),
                    };
                },
                [](BindingType::AccelerationStructure&&) {
                    PANIC("TODO!");
                },
                [](auto&&) {},
            }, entry.Type);

            return wgpu_entry;
        });

        WGPUBindGroupLayoutDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Bind Group"),
            .entryCount = CAST(u32, entries.size()),
            .entries = entries.data(),
        };
        auto bg_layout = wgpuDeviceCreateBindGroupLayout(CAST(WGPUDevice, Handle), &desc);
        return BindGroupLayout{ bg_layout };
    }

    auto Device::CreateShaderModule(ShaderModuleSpec const& spec) const -> ShaderModule
    {
        WGPUShaderModuleWGSLDescriptor wgsl_descriptor{
            .chain = {
                .next = nullptr,
                .sType = WGPUSType_ShaderModuleWGSLDescriptor,
            },
            .code = spec.Source.data(),
        };

        WGPUShaderModuleDescriptor desc{
            .nextInChain = &wgsl_descriptor.chain,
            .label = "Shader",
            .hintCount = 0,
            .hints = nullptr,
        };

        auto module = wgpuDeviceCreateShaderModule(CAST(WGPUDevice, Handle), &desc);

        return ShaderModule{ module, spec };
    }

    auto Device::CreatePipelineLayout(PipelineLayoutSpec const& spec) const -> PipelineLayout
    {
        std::vector<WGPUBindGroupLayout> layouts{};
        std::ranges::transform(spec.BindGroupLayouts, std::back_inserter(layouts), [](BindGroupLayout const& bind_group_layout) {
            return CAST(WGPUBindGroupLayout, bind_group_layout.Handle);
        });

        WGPUPipelineLayoutDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Pipeline Layout"),
            .bindGroupLayoutCount = CAST(u32, layouts.size()),
            .bindGroupLayouts = layouts.data(),
        };

        auto layout = wgpuDeviceCreatePipelineLayout(CAST(WGPUDevice, Handle), &desc);
        return PipelineLayout{ layout };
    }

    auto Device::CreateRenderPipeline(ShaderModule const& module, RenderPipelineSpec const& spec) const -> RenderPipeline
    {
        std::vector<WGPUVertexBufferLayout> vertex_buffer_layouts{};
        std::ranges::transform(spec.Vertex.AttributeLayouts, std::back_inserter(vertex_buffer_layouts), [](VertexBufferLayout const& layout) {
            std::vector<WGPUVertexAttribute> attributes{};
            std::ranges::transform(layout.Attributes, std::back_inserter(attributes), [](VertexAttribute const& attribute) {
                return WGPUVertexAttribute{
                    .format = ToWgpu(attribute.Type),
                    .offset = attribute.Offset,
                    .shaderLocation = attribute.ShaderLocation,
                };
            });

            WGPUVertexBufferLayout wgpu_layout{
                .arrayStride = layout.ArrayStride,
                .stepMode = ToWgpu(layout.StepMode),
                .attributeCount = CAST(u32, attributes.size()),
                .attributes = attributes.data(),
            };

            return wgpu_layout;
        });

        WGPUVertexState vertex{
            .nextInChain = nullptr,
            .module = module.As<WGPUShaderModule>(),
            .entryPoint = module.Spec.VertexEntryPoint.data(),
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = CAST(u32, vertex_buffer_layouts.size()),
            .buffers = vertex_buffer_layouts.data(),
        };

        WGPUPrimitiveState primitive{
            .nextInChain = nullptr,
            .topology = ToWgpu(spec.Primitive.Topology),
            .stripIndexFormat = WGPUIndexFormat_Undefined,
            .frontFace = ToWgpu(spec.Primitive.FrontFace),
            .cullMode = ToWgpu(spec.Primitive.Cull),
        };

        if (spec.Primitive.StripIndexFormat) {
            primitive.stripIndexFormat = ToWgpu(*spec.Primitive.StripIndexFormat);
        }

        WGPUMultisampleState multisample{
            .nextInChain = nullptr,
            .count = spec.MultiSample.Count,
            .mask = spec.MultiSample.Mask,
            .alphaToCoverageEnabled = spec.MultiSample.AlphaToCoverageEnabled,
        };

        std::vector<WGPUColorTargetState> color_targets{};
        // We need to save these here because the color
        // target state gets a pointer to the blend state.
        std::vector<WGPUBlendState> blends{};

        std::ranges::transform(spec.Fragment.Targets, std::back_inserter(color_targets), [&blends](ColorTargetState const& state) {
            WGPUColorTargetState wgpu_state{
                .nextInChain = nullptr,
                .format = ToWgpu(state.Format),
                .writeMask = ToWgpu(state.WriteMask),
            };

            if (auto b = state.Blend) {
                auto& blend = blends.emplace_back();
                blend = {
                    .color = {
                        .operation = ToWgpu(b->Color.Operation),
                        .srcFactor = ToWgpu(b->Color.SrcFactor),
                        .dstFactor = ToWgpu(b->Color.DstFactor)
                    },
                    .alpha = {
                        .operation = ToWgpu(b->Alpha.Operation),
                        .srcFactor = ToWgpu(b->Alpha.SrcFactor),
                        .dstFactor = ToWgpu(b->Alpha.DstFactor)
                    }
                };

                wgpu_state.blend = &blend;
            }

            return wgpu_state;
        });

        WGPUFragmentState fragment{
            .nextInChain = nullptr,
            .module = module.As<WGPUShaderModule>(),
            .entryPoint = module.Spec.FragmentEntryPoint.data(),
            .constantCount = 0,
            .constants = nullptr,
            .targetCount = CAST(u32, color_targets.size()),
            .targets = color_targets.data(),
        };

        WGPURenderPipelineDescriptor desc{
            .nextInChain = nullptr,
            .label = spec.Label.ValueOr("Render Pipeline"),
            .vertex = vertex,
            .primitive = primitive,
            .depthStencil = nullptr,
            .multisample = multisample,
            .fragment = &fragment,
        };
        if (spec.Layout.HasValue()) {
            desc.layout = spec.Layout->As<WGPUPipelineLayout>();
        }

        WGPUDepthStencilState depth_stencil;
        if (spec.DepthStencil) {
            depth_stencil = {
                .nextInChain = nullptr,
                .format = ToWgpu(spec.DepthStencil->Format),
                .depthWriteEnabled = spec.DepthStencil->DepthWriteEnabled,
                .depthCompare = ToWgpu(spec.DepthStencil->DepthCompare),
                .stencilFront = {
                    .compare = ToWgpu(spec.DepthStencil->Stencil.Front.Compare),
                    .failOp = ToWgpu(spec.DepthStencil->Stencil.Front.FailOp),
                    .depthFailOp = ToWgpu(spec.DepthStencil->Stencil.Front.DepthFailOp),
                    .passOp = ToWgpu(spec.DepthStencil->Stencil.Front.PassOp),
                },
                .stencilBack = {
                    .compare = ToWgpu(spec.DepthStencil->Stencil.Back.Compare),
                    .failOp = ToWgpu(spec.DepthStencil->Stencil.Back.FailOp),
                    .depthFailOp = ToWgpu(spec.DepthStencil->Stencil.Back.DepthFailOp),
                    .passOp = ToWgpu(spec.DepthStencil->Stencil.Back.PassOp),
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
        return RenderPipeline{ pipeline };
    }

    void Device::SubmitCommandBuffer(CommandBuffer cmd) const
    {
        WGPUCommandBuffer cmds[] = {
            CAST(WGPUCommandBuffer, cmd.Handle),
        };
        wgpuQueueSubmit(CAST(WGPUQueue, Queue), 1, cmds);
    }

    void Device::WriteTexture(
        Texture const& texture,
        void const* data,
        size_t data_size,
        Vector2 const& origin,
        Vector2 const& size,
        u32 mip_level) const
    {
        WGPUImageCopyTexture copy_texture{
            .nextInChain = nullptr,
            .texture = CAST(WGPUTexture, texture.Handle),
            .mipLevel = mip_level,
            .origin = {
                .x = CAST(u32, origin.X),
                .y = CAST(u32, origin.Y),
                .z = 0,
            },
            .aspect = WGPUTextureAspect_All,
        };

        WGPUTextureDataLayout layout{
            .nextInChain = nullptr,
            .offset = 0,
            .bytesPerRow = CAST(u32, size.X * 4),
            .rowsPerImage = CAST(u32, size.Y),
        };

        WGPUExtent3D texture_size{
            .width = CAST(u32, texture.Spec.Size.X),
            .height = CAST(u32, texture.Spec.Size.Y),
            .depthOrArrayLayers = 1,
        };
        wgpuQueueWriteTexture(CAST(WGPUQueue, Queue), &copy_texture, data, data_size, &layout, &texture_size);

    }

    void Device::WriteBuffer(Buffer const& buffer, u64 offset, void const* data, size_t size) const
    {
        wgpuQueueWriteBuffer(CAST(WGPUQueue, Queue), CAST(WGPUBuffer, buffer.Handle), offset, data, size);
    }

    void Device::SetErrorCallback(ErrorFn const& function)
    {
        m_Function = std::move(function);

        wgpuDeviceSetUncapturedErrorCallback(CAST(WGPUDevice, Handle), [](WGPUErrorType type, char const* message, void* userdata) {
            auto self = CAST(Device*, userdata);

            if (self->m_Function)
                self->m_Function(FromWgpu(type), std::string_view(message));
        }, this);
    }

    auto Adapter::GetDevice() -> Device
    {
        WGPUDeviceDescriptor desc{
            .nextInChain = nullptr,
            .label = "Device",
            // .requiredFeatureCount = ,
            // .requiredFeatures = ,
            // .requiredLimits = ,
            .defaultQueue = {
                .nextInChain = nullptr,
                .label = "Default Queue"
            },
            .deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
                PANIC("Device lost: {}\n\tMessage: {}", magic_enum::enum_name(reason), message);
            },
            .deviceLostUserdata = this,
        };

        auto device = RequestDeviceSync(CAST(WGPUAdapter, Handle), &desc);

        return Device{ device };
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
        WGPUTextureFormat surface_format = wgpuSurfaceGetPreferredFormat(CAST(WGPUSurface, Handle), CAST(WGPUAdapter, adapter.Handle));
        WGPUSurfaceConfiguration conf{
            .nextInChain = nullptr,
            .device = CAST(WGPUDevice, device.Handle),
            .format = surface_format,
            .usage = WGPUTextureUsage_RenderAttachment,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
            .alphaMode = WGPUCompositeAlphaMode_Auto,
            .width = CAST(u32, config.Size.X),
            .height = CAST(u32, config.Size.Y),
            .presentMode = ToWgpu(config.PresentMode),
        };
        wgpuSurfaceConfigure(CAST(WGPUSurface, Handle), &conf);

        Format = FromWgpu(surface_format);
    }

    auto Surface::GetNextView() -> TextureView
    {
        WGPUSurfaceTexture texture;
        wgpuSurfaceGetCurrentTexture(CAST(WGPUSurface, Handle), &texture);

        if (texture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
            return {};
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

    void Surface::Present() const
    {
        wgpuSurfacePresent(CAST(WGPUSurface, Handle));
    }

    Instance Instance::Create(InstanceSpec const& spec)
    {
        Instance instance;
        WGPUInstanceExtras instance_extras{
            .chain = {
                .next = nullptr,
                .sType = CAST(WGPUSType, WGPUSType_InstanceExtras),
            },
            .backends = ToWgpu(spec.Backend),
        };

        WGPUInstanceDescriptor desc{
            .nextInChain = &instance_extras.chain
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
        return Surface{ surface };
    }

    auto Instance::GetAdapter(Surface surface, AdapterOptions const& opt) const -> Adapter
    {
        WGPURequestAdapterOptions options{
            .nextInChain = nullptr,
            .compatibleSurface = CAST(WGPUSurface, surface.Handle),
            .powerPreference = ToWgpu(opt.PowerPreference),
        };
        auto adapter = RequestAdapterSync(CAST(WGPUInstance, Handle), &options);
        return Adapter{ adapter };
    }
}
