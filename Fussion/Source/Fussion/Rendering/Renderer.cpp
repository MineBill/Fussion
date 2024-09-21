#include "FussionPCH.h"
#include "Renderer.h"

#include "Fussion/Util/TextureImporter.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/GPU/Utils.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Renderer* Renderer::s_renderer = nullptr;

    void Renderer::initialize(Window const& window)
    {
        LOG_INFO("Initializing Renderer");
        s_renderer = new Renderer();

        // auto instance = Instance::Create(window);
        // auto* device = Device::Create(instance);
        // Device::SetInstance(device);

        auto instance = GPU::Instance::create({
            .backend = GPU::BackendRenderer::Vulkan,
        });

        auto surface = instance.surface(&window);
        auto adapter = instance.adapter(surface, {
            .power_preference = GPU::DevicePower::HighPerformance
        });

        GPU::DeviceSpec spec{
            .label = String("Device"),
            .required_features = {
                GPU::Features::Float32Filterable,
                GPU::Features::TimestampQuery,
                GPU::Features::PipelineStatistics,
            }
        };
        auto device = adapter.device(spec);

        s_renderer->m_window_size = window.size();

        surface.configure(device, adapter, {
            .present_mode = GPU::PresentMode::Immediate,
            .size = s_renderer->m_window_size
        });

        s_renderer->m_device = device;
        s_renderer->m_adapter = adapter;
        s_renderer->m_instance = instance;
        s_renderer->m_surface = surface;

        // NOTE: Setting the callback on m_Device, rather than the device local variable, because
        //       the GPU::Device will set up WGPU user data with its pointer.
        // s_Renderer->m_Device.SetErrorCallback([&](GPU::ErrorType type, std::string_view message) {
        //     LOG_ERRORF("!DEVICE ERROR!\n\tTYPE: {}\n\tMESSAGE: {}", magic_enum::enum_name(type), message);
        // });

        GPU::Utils::RenderDoc::initialize();
    }

    void Renderer::shutdown()
    {
        LOG_DEBUGF("Shutting down Renderer!");

        s_renderer->m_device.release();
        s_renderer->m_adapter.release();
        s_renderer->m_surface.release();
        s_renderer->m_instance.release();
    }

    auto Renderer::begin_rendering() -> Maybe<GPU::TextureView>
    {
        ZoneScoped;
        auto new_size = Application::inst()->window().size();
        if (s_renderer->m_window_size != new_size) {
            resize(new_size);
        }

        if (s_renderer->m_skip_render)
            return None();

        auto view = s_renderer->m_surface.get_next_view();
        if (view.is_error()) {
            // Handle resize or the reason the view is empty
            // ...
            return None();
        }
        return view.value();
    }

    void Renderer::end_rendering(GPU::CommandBuffer cmd)
    {
        ZoneScoped;
        s_renderer->m_device.submit_command_buffer(cmd);
        s_renderer->m_surface.present();
        cmd.release();
    }

    void Renderer::resize(Vector2 const& new_size)
    {
        ZoneScoped;
        if (new_size.is_zero()) {
            s_renderer->m_skip_render = true;
            return;
        }
        s_renderer->m_skip_render = false;

        s_renderer->m_window_size = new_size;
        s_renderer->m_surface.configure(s_renderer->m_device, s_renderer->m_adapter, {
            .present_mode = GPU::PresentMode::Immediate,
            .size = new_size
        });
    }

    auto Renderer::default_normal_map() -> AssetRef<Texture2D> { return s_renderer->m_normal_map; }

    auto Renderer::white_texture() -> AssetRef<Texture2D> { return s_renderer->m_white_texture; }

    auto Renderer::black_texture() -> AssetRef<Texture2D> { return s_renderer->m_black_texture; }

    static unsigned char g_white_texture_png[] = {
#include "white_texture.png.h"
    };

    static unsigned char g_black_texture_png[] = {
#include "black_texture.png.h"
    };

    static unsigned char g_normal_map_png[] = {
#include "default_normal_map.png.h"
    };

    void Renderer::create_default_resources()
    {
        auto material = make_ref<PbrMaterial>();
        material->object_color = Color(1, 1, 1, 1);
        s_renderer->m_default_material = AssetManager::create_virtual_asset_ref<PbrMaterial>(material);

        s_renderer->m_white_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureImporter::load_texture_from_memory(g_white_texture_png).value(), "Default White Texture");
        s_renderer->m_black_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureImporter::load_texture_from_memory(g_black_texture_png).value(), "Default Black Texture");
        s_renderer->m_normal_map = AssetManager::create_virtual_asset_ref<Texture2D>(TextureImporter::load_texture_from_memory(g_normal_map_png, true).value(), "Default Normal Map");

    }
}
