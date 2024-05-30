int main()
{
    //
    // using namespace Engin5;
    // Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);
    //
    // const auto window = Window::Create(WindowFlag::Resizable);
    //
    // Renderer::Init(*window);
    //
    // auto device = Device::Instance();
    //
    // auto spec = RenderPassSpecification {
    //     .Label = "Main RenderPass",
    //     .Attachments = {
    //         RenderPassAttachment {
    //             .Label = "Color Attachment",
    //             .LoadOp = RenderPassAttachmentLoadOp::Clear,
    //             .StoreOp = RenderPassAttachmentStoreOp::Store,
    //             .Format = ImageFormat::B8G8R8A8_UNORM,
    //             .FinalLayout = ImageLayout::PresentSrc,
    //             .ClearColor = {0.2f, 0.6f, 0.15f, 1.0f},
    //         },
    //         RenderPassAttachment {
    //             .Label = "Depth Attachment",
    //             .LoadOp = RenderPassAttachmentLoadOp::Clear,
    //             .Format = ImageFormat::D32_SFLOAT,
    //             .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
    //             .ClearDepth = 1.f,
    //         }
    //     },
    //     .SubPasses = {
    //         RenderPassSubPass {
    //             .ColorAttachments = {
    //                 {
    //                     .Attachment = 0,
    //                     .Layout = ImageLayout::ColorAttachmentOptimal,
    //                 }
    //             },
    //             .DepthStencilAttachment = RenderPassAttachmentRef {
    //                 .Attachment = 1,
    //                 .Layout = ImageLayout::DepthStencilAttachmentOptimal,
    //             }
    //         },
    //     }
    // };
    // auto pass = device->CreateRenderPass(spec);
    //
    // auto swapchain_spec = SwapChainSpecification {
    //     .Size = {400, 400},
    //     .VideoPresentMode = VideoPresentMode::Immediate,
    //     .Format = ImageFormat::B8G8R8A8_UNORM,
    // };
    // auto swapchain = device->CreateSwapchain(pass, swapchain_spec);
    //
    // auto cmd_spec = CommandBufferSpecification {
    //     .Label = "Pepegas",
    // };
    // auto command_buffers = device->CreateCommandBuffers(3, cmd_spec);
    //
    //
    // LOG_INFOF("Hello, {}", "world");
    //
    // Vector2 window_size = {window->GetWidth(), window->GetHeight()};
    // window->OnEvent([&](Event& event) -> bool {
    //     EventDispatcher dispatcher(event);
    //     dispatcher.Dispatch<WindowResized>([&](const WindowResized& resized) -> bool {
    //         window_size = {resized.Width, resized.Height};
    //         return true;
    //     });
    //     return false;
    // });
    //
    // LOG_DEBUGF("Current path is {}", std::filesystem::current_path().string());
    // const auto shader = FileSystem::ReadEntireFile("Assets/triangle.shader");
    //
    // auto [stages, metadata] = ShaderCompiler::Compile(shader);
    //
    // for (const auto& [set, bindings] : metadata.Uniforms) {
    //     for(const auto& [binding, usage] : bindings) {
    //         LOG_DEBUGF("Set {}, Binding {}, {}", set, binding, usage.Label);
    //     }
    // }
    //
    // auto s = device->CreateShader(pass, stages, metadata);
    //
    // const auto buffer_spec = BufferSpecification {
    //     .Label = "Simple Vertex Bufer",
    //     .Usage = BufferUsage::Vertex,
    //     .Size = sizeof(Vector3) * 3,
    //     .Mapped = true,
    // };
    // auto buffer = device->CreateBuffer(buffer_spec);
    //
    // std::vector triangle_vertices = {
    //     Vector3( 0.0,  0.5, 0.0),
    //     Vector3( 0.5, -0.5, 0.0),
    //     Vector3(-0.5, -0.5, 0.0)
    // };
    // // buffer->SetData({transmute(s8*, triangle_vertices.data()), triangle_vertices.size() * 4});
    // buffer->SetData(triangle_vertices.data(), triangle_vertices.size() * sizeof(Vector3));
    //
    // while (!window->ShouldClose()) {
    //     window->Update();
    //
    //     auto [image, ok] = swapchain->GetNextImage();
    //     if (!ok) {
    //         // @note Is there a better way to handle resizing?
    //         device->WaitIdle();
    //         swapchain->Resize(window->GetWidth(), window->GetHeight());
    //         continue;
    //     }
    //
    //     auto cmd = command_buffers[image];
    //     cmd->Begin();
    //
    //     cmd->BeginRenderPass(pass, swapchain->GetFrameBuffer(image));
    //
    //     cmd->SetViewport({window_size.x, -window_size.y});
    //     cmd->SetScissor({0, 0, window_size.x, window_size.y});
    //
    //     cmd->UseShader(s);
    //
    //     cmd->BindBuffer(buffer);
    //     cmd->Draw(3, 1);
    //
    //     cmd->EndRenderPass(pass);
    //
    //     cmd->End();
    //
    //     swapchain->SubmitCommandBuffer(cmd);
    //     swapchain->Present(image);
    //
    //     FrameMark;
    // }
}