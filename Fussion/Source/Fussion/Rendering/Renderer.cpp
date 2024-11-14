#include "FussionPCH.h"
#include "Renderer.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Core/Application.h"
#include "Fussion/GPU/Utils.h"
#include "Fussion/Util/TextureLoader.h"
#include "Pipelines/IrradianceIBLGenerator.h"

#include <tracy/Tracy.hpp>

#ifdef IS_XMAKE
static unsigned char g_white_texture_png[] = {
#    include "white_texture.png.h"
};

static unsigned char g_white_texture_hdr[] = {
#    include "white_texture.hdr.h"
};

static unsigned char g_black_texture_png[] = {
#    include "black_texture.png.h"
};

static unsigned char g_normal_map_png[] = {
#    include "default_normal_map.png.h"
};
#else
#    include "battery/embed.hpp"
#endif

namespace Fussion {
    struct Data {
        AssetRef<PbrMaterial> default_material;
        AssetRef<Texture2D> white_texture, black_texture, normal_map;
        GPU::Texture white_cube_texture;

        Vector2 window_size {};
        bool skip_render {};

        GPU::Instance instance {};
        GPU::Device device {};
        GPU::Adapter adapter {};
        GPU::Surface surface {};
        bool supports_pipeline_statistics {};

        GPU::PresentMode present_mode {};
        GPU::TextureFormat surface_format {};

        IrradianceIBLGenerator irradiance_generator;
    } g_data;

    void Renderer::initialize(Window const& window)
    {
        LOG_INFO("Initializing Renderer");
        GPU::ShaderProcessor::initialize();

        auto instance = GPU::Instance::create({
            .backend = GPU::BackendRenderer::Vulkan,
        });

        auto surface = instance.surface(&window);
        auto adapter = instance.request_adapter(
            surface,
            {
                .power_preference = GPU::DevicePower::HighPerformance,
            }
        );

        GPU::DeviceSpec spec {
            .label = String("Device"),
            .required_features = {
                GPU::Feature::Float32Filterable,
                GPU::Feature::TimestampQuery,
                GPU::Feature::SpirVPassthrough,
            }
        };

        if (adapter.has_feature(GPU::Feature::PipelineStatistics)) {
            g_data.supports_pipeline_statistics = true;
            spec.required_features.push_back(GPU::Feature::PipelineStatistics);
        }
        auto device = adapter.request_device(spec);

        g_data.window_size = window.size();

        auto caps = surface.capabilities(adapter);
        VERIFY(!caps.available_surface_formats.empty());
        g_data.surface_format = caps.available_surface_formats[0];

        std::array present_modes {
            GPU::PresentMode::Immediate,
            GPU::PresentMode::FifoRelaxed,
            GPU::PresentMode::Fifo,
            GPU::PresentMode::Mailbox,
        };

        Maybe<GPU::PresentMode> present_mode;
        for (auto const& mode : present_modes) {
            if (std::ranges::contains(caps.available_present_modes, mode)) {
                present_mode = mode;
                break;
            }
        }
        g_data.present_mode = present_mode.value_or(caps.available_present_modes[0]);
        LOG_INFOF("Choosing '{}' present mode", magic_enum::enum_name(g_data.present_mode));

        if (!std::ranges::contains(caps.available_present_modes, GPU::PresentMode::Immediate)) {
            g_data.present_mode = caps.available_present_modes[0];
        }

        surface.configure(device, g_data.surface_format, { .present_mode = g_data.present_mode, .size = g_data.window_size });

        g_data.device = device;
        g_data.adapter = adapter;
        g_data.instance = instance;
        g_data.surface = surface;

        g_data.irradiance_generator.init();

        GPU::Utils::RenderDoc::initialize();
    }

    void Renderer::shutdown()
    {
        LOG_DEBUGF("Shutting down Renderer!");
        GPU::ShaderProcessor::shutdown();

        g_data.device.release();
        g_data.adapter.release();
        g_data.surface.release();
        g_data.instance.release();
    }

    auto Renderer::begin_rendering() -> Maybe<GPU::TextureView>
    {
        ZoneScoped;
        auto new_size = Application::self()->window().size();
        if (g_data.window_size != new_size) {
            resize(new_size);
        }

        if (g_data.skip_render)
            return None();

        auto view = g_data.surface.next_view();
        if (view.has_error()) {
            // Handle resize or the reason the view is empty
            // ...
            return None();
        }
        return view.unwrap();
    }

    void Renderer::end_rendering(GPU::CommandBuffer cmd)
    {
        ZoneScoped;
        g_data.device.submit_command_buffer(cmd);
        g_data.surface.present();
    }

    void Renderer::resize(Vector2 const& new_size)
    {
        ZoneScoped;
        if (new_size.is_zero()) {
            g_data.skip_render = true;
            return;
        }
        g_data.skip_render = false;

        g_data.window_size = new_size;
        g_data.surface.configure(g_data.device, g_data.surface_format, { .present_mode = g_data.present_mode, .size = new_size });
    }

    auto Renderer::device() -> GPU::Device&
    {
        return g_data.device;
    }

    auto Renderer::surface() -> GPU::Surface&
    {
        return g_data.surface;
    }

    auto Renderer::gpu_instance() -> GPU::Instance&
    {
        return g_data.instance;
    }

    auto Renderer::default_material() -> AssetRef<PbrMaterial>
    {
        return g_data.default_material;
    }

    auto Renderer::default_normal_map() -> AssetRef<Texture2D> { return g_data.normal_map; }

    auto Renderer::white_texture() -> AssetRef<Texture2D> { return g_data.white_texture; }

    auto Renderer::black_texture() -> AssetRef<Texture2D> { return g_data.black_texture; }

    auto Renderer::white_cube_texture() -> GPU::Texture
    {
        return g_data.white_cube_texture;
    }

    void Renderer::create_default_resources()
    {
        auto material = make_ref<PbrMaterial>();
        material->object_color = Color(1, 1, 1, 1);
        g_data.default_material = AssetManager::create_virtual_asset_ref<PbrMaterial>(material);

#ifndef IS_XMAKE
        auto g_white_texture_png = b::embed<"Assets/Textures/white_texture.png">().vec();
        auto g_black_texture_png = b::embed<"Assets/Textures/black_texture.png">().vec();
        auto g_normal_map_png = b::embed<"Assets/Textures/default_normal_map.png">().vec();
        auto g_white_texture_hdr = b::embed<"Assets/Textures/white_texture.hdr">().vec();
#endif
        g_data.white_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_white_texture_png, GPU::TextureFormat::RGBA8UnormSrgb).unwrap(), "Default White Texture");
        g_data.black_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_black_texture_png, GPU::TextureFormat::RGBA8UnormSrgb).unwrap(), "Default Black Texture");
        g_data.normal_map = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_normal_map_png, GPU::TextureFormat::RGBA8Unorm, true).unwrap(), "Default Normal Map");

        GPU::TextureSpec texture_spec {
            .label = "CubeTexGen::cube_texture"sv,
            .usage = GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopyDst,
            .dimension = GPU::TextureDimension::D2,
            .size = { 512, 512, 6 },
            .format = GPU::TextureFormat::RGBA16Float,
            .sample_count = 1,
            .aspect = GPU::TextureAspect::All,
            .generate_mip_maps = false,
            .initialize_view = false,
        };

        g_data.white_cube_texture = device().create_texture(texture_spec);
        g_data.white_cube_texture.view = g_data.white_cube_texture.create_view({
            .label = "View"sv,
            .usage = texture_spec.usage,
            .dimension = GPU::TextureViewDimension::Cube, // TODO: Make configurable
            .format = texture_spec.format,
            .base_mip_level = 0, // TODO: Make configurable
            .mip_level_count = 1,
            .base_array_layer = 0,          // TODO: Make configurable
            .array_layer_count = 6,         // TODO: Make configurable
            .aspect = texture_spec.aspect // TODO: Make configurable
        });

        auto data = TextureLoader::load_hdr_texture_from_memory(g_white_texture_hdr).unwrap();

        auto encoder = g_data.device.create_command_encoder();

        for (u32 i = 0; i < 6; ++i) {
            encoder.copy_texture_to_texture(data.get()->texture(), g_data.white_cube_texture, { 1, 1 }, 0, 0, 0, i);
        }
        g_data.device.submit_command_buffer(encoder.finish());
        encoder.release();
    }

    bool Renderer::supports_pipeline_statistics()
    {
        return g_data.supports_pipeline_statistics;
    }

    GPU::Texture Renderer::generate_irradiance_map(GPU::Texture const& texture)
    {
        return g_data.irradiance_generator.generate(texture);
    }
}
