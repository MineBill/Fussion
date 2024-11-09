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
        GPU::ShaderProcessor::Initialize();

        auto instance = GPU::Instance::Create({
            .Backend = GPU::BackendRenderer::Vulkan,
        });

        auto surface = instance.GetSurface(&window);
        auto adapter = instance.GetAdapter(
            surface,
            {
                .PowerPreference = GPU::DevicePower::HighPerformance,
            }
        );

        GPU::DeviceSpec spec {
            .Label = String("Device"),
            .RequiredFeatures = {
                GPU::Feature::Float32Filterable,
                GPU::Feature::TimestampQuery,
                GPU::Feature::SpirVPassthrough,
            }
        };

        if (adapter.HasFeature(GPU::Feature::PipelineStatistics)) {
            g_data.supports_pipeline_statistics = true;
            spec.RequiredFeatures.push_back(GPU::Feature::PipelineStatistics);
        }
        auto device = adapter.RequestDevice(spec);

        g_data.window_size = window.size();

        auto caps = surface.GetCapabilities(adapter);
        VERIFY(!caps.AvailableSurfaceFormats.empty());
        g_data.surface_format = caps.AvailableSurfaceFormats[0];

        std::array present_modes {
            GPU::PresentMode::Immediate,
            GPU::PresentMode::FifoRelaxed,
            GPU::PresentMode::Fifo,
            GPU::PresentMode::Mailbox,
        };

        Maybe<GPU::PresentMode> present_mode;
        for (auto const& mode : present_modes) {
            if (std::ranges::contains(caps.AvailablePresentModes, mode)) {
                present_mode = mode;
                break;
            }
        }
        g_data.present_mode = present_mode.value_or(caps.AvailablePresentModes[0]);
        LOG_INFOF("Choosing '{}' present mode", magic_enum::enum_name(g_data.present_mode));

        if (!std::ranges::contains(caps.AvailablePresentModes, GPU::PresentMode::Immediate)) {
            g_data.present_mode = caps.AvailablePresentModes[0];
        }

        surface.Configure(device, g_data.surface_format, { .Mode = g_data.present_mode, .Size = g_data.window_size });

        g_data.device = device;
        g_data.adapter = adapter;
        g_data.instance = instance;
        g_data.surface = surface;

        g_data.irradiance_generator.init();

        GPU::Utils::RenderDoc::Initialize();
    }

    void Renderer::shutdown()
    {
        LOG_DEBUGF("Shutting down Renderer!");
        GPU::ShaderProcessor::Shutdown();

        g_data.device.Release();
        g_data.adapter.Release();
        g_data.surface.Release();
        g_data.instance.Release();
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

        auto view = g_data.surface.GetNextView();
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
        g_data.device.SubmitCommandBuffer(cmd);
        g_data.surface.Present();
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
        g_data.surface.Configure(g_data.device, g_data.surface_format, { .Mode = g_data.present_mode, .Size = new_size });
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
        g_data.white_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_white_texture_png).unwrap(), "Default White Texture");
        g_data.black_texture = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_black_texture_png).unwrap(), "Default Black Texture");
        g_data.normal_map = AssetManager::create_virtual_asset_ref<Texture2D>(TextureLoader::load_texture_from_memory(g_normal_map_png, true).unwrap(), "Default Normal Map");

        GPU::TextureSpec texture_spec {
            .Label = "CubeTexGen::cube_texture"sv,
            .Usage = GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopyDst,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { 512, 512, 6 },
            .Format = GPU::TextureFormat::RGBA16Float,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
            .GenerateMipMaps = false,
            .InitializeView = false,
        };

        g_data.white_cube_texture = device().CreateTexture(texture_spec);
        g_data.white_cube_texture.View = g_data.white_cube_texture.CreateView({
            .Label = "View"sv,
            .Usage = texture_spec.Usage,
            .Dimension = GPU::TextureViewDimension::Cube, // TODO: Make configurable
            .Format = texture_spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = 1,
            .BaseArrayLayer = 0,          // TODO: Make configurable
            .ArrayLayerCount = 6,         // TODO: Make configurable
            .Aspect = texture_spec.Aspect // TODO: Make configurable
        });

        auto data = TextureLoader::load_hdr_texture_from_memory(g_white_texture_hdr).unwrap();

        auto encoder = g_data.device.CreateCommandEncoder();

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(data.get()->texture(), g_data.white_cube_texture, { 1, 1 }, 0, 0, 0, i);
        }
        g_data.device.SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
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
