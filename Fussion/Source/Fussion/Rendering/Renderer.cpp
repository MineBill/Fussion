#include "FussionPCH.h"
#include "Renderer.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Core/Application.h"
#include "Fussion/GPU/Utils.h"
#include "Fussion/Util/TextureImporter.h"
#include "Pipelines/IrradianceIBLGenerator.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    struct Data {
        AssetRef<PbrMaterial> DefaultMaterial;
        AssetRef<Texture2D> WhiteTexture, BlackTexture, NormalMap;
        GPU::Texture WhiteCubeTexture;

        Vector2 WindowSize {};
        bool SkipRender {};

        GPU::Instance Instance {};
        GPU::Device Device {};
        GPU::Adapter Adapter {};
        GPU::Surface Surface {};
        bool HasPipelineStatistics {};

        IrradianceIBLGenerator IrradianceGenerator;
    } g_Data;

    void Renderer::Initialize(Window const& window)
    {
        LOG_INFO("Initializing Renderer");

        auto instance = GPU::Instance::Create({
            .Backend = GPU::BackendRenderer::Vulkan,
        });

        auto surface = instance.GetSurface(&window);
        auto adapter = instance.GetAdapter(surface,
            {
                .PowerPreference = GPU::DevicePower::HighPerformance,
            });

        GPU::DeviceSpec spec {
            .Label = String("Device"),
            .RequiredFeatures = {
                GPU::Feature::Float32Filterable,
                GPU::Feature::TimestampQuery,
                GPU::Feature::SpirVPassthrough,
            }
        };

        if (adapter.HasFeature(GPU::Feature::PipelineStatistics)) {
            g_Data.HasPipelineStatistics = true;
            spec.RequiredFeatures.push_back(GPU::Feature::PipelineStatistics);
        }
        auto device = adapter.RequestDevice(spec);

        g_Data.WindowSize = window.Size();

        surface.Configure(device, adapter, { .Mode = GPU::PresentMode::Fifo, .Size = g_Data.WindowSize });

        g_Data.Device = device;
        g_Data.Adapter = adapter;
        g_Data.Instance = instance;
        g_Data.Surface = surface;

        g_Data.IrradianceGenerator.Initialize();

        GPU::Utils::RenderDoc::Initialize();
    }

    void Renderer::Shutdown()
    {
        LOG_DEBUGF("Shutting down Renderer!");

        g_Data.Device.Release();
        g_Data.Adapter.Release();
        g_Data.Surface.Release();
        g_Data.Instance.Release();
    }

    auto Renderer::BeginRendering() -> Maybe<GPU::TextureView>
    {
        ZoneScoped;
        auto new_size = Application::Self()->GetWindow().Size();
        if (g_Data.WindowSize != new_size) {
            Resize(new_size);
        }

        if (g_Data.SkipRender)
            return None();

        auto view = g_Data.Surface.GetNextView();
        if (view.HasError()) {
            // Handle resize or the reason the view is empty
            // ...
            return None();
        }
        return view.Unwrap();
    }

    void Renderer::EndRendering(GPU::CommandBuffer cmd)
    {
        ZoneScoped;
        g_Data.Device.SubmitCommandBuffer(cmd);
        g_Data.Surface.Present();
    }

    void Renderer::Resize(Vector2 const& new_size)
    {
        ZoneScoped;
        if (new_size.IsZero()) {
            g_Data.SkipRender = true;
            return;
        }
        g_Data.SkipRender = false;

        g_Data.WindowSize = new_size;
        g_Data.Surface.Configure(g_Data.Device, g_Data.Adapter, { .Mode = GPU::PresentMode::Immediate, .Size = new_size });
    }

    auto Renderer::Device() -> GPU::Device&
    {
        return g_Data.Device;
    }

    auto Renderer::Surface() -> GPU::Surface&
    {
        return g_Data.Surface;
    }

    auto Renderer::GPUInstance() -> GPU::Instance&
    {
        return g_Data.Instance;
    }

    auto Renderer::DefaultMaterial() -> AssetRef<PbrMaterial>
    {
        return g_Data.DefaultMaterial;
    }

    auto Renderer::DefaultNormalMap() -> AssetRef<Texture2D> { return g_Data.NormalMap; }

    auto Renderer::WhiteTexture() -> AssetRef<Texture2D> { return g_Data.WhiteTexture; }

    auto Renderer::BlackTexture() -> AssetRef<Texture2D> { return g_Data.BlackTexture; }

    auto Renderer::WhiteCubeTexture() -> GPU::Texture
    {
        return g_Data.WhiteCubeTexture;
    }

    static unsigned char g_white_texture_png[] = {
#include "white_texture.png.h"
    };

    static unsigned char g_white_texture_hdr[] = {
#include "white_texture.hdr.h"
    };

    static unsigned char g_black_texture_png[] = {
#include "black_texture.png.h"
    };

    static unsigned char g_normal_map_png[] = {
#include "default_normal_map.png.h"
    };

    void Renderer::CreateDefaultResources()
    {
        auto material = MakeRef<PbrMaterial>();
        material->object_color = Color(1, 1, 1, 1);
        g_Data.DefaultMaterial = AssetManager::CreateVirtualAssetRef<PbrMaterial>(material);

        g_Data.WhiteTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_white_texture_png).Unwrap(), "Default White Texture");
        g_Data.BlackTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_black_texture_png).Unwrap(), "Default Black Texture");
        g_Data.NormalMap = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_normal_map_png, true).Unwrap(), "Default Normal Map");

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

        g_Data.WhiteCubeTexture = Device().CreateTexture(texture_spec);
        g_Data.WhiteCubeTexture.View = g_Data.WhiteCubeTexture.CreateView({
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

        auto data = TextureImporter::LoadHDRTextureFromMemory(g_white_texture_hdr).Unwrap();

        auto encoder = g_Data.Device.CreateCommandEncoder();

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(data.get()->GetTexture(), g_Data.WhiteCubeTexture, { 1, 1 }, 0, 0, 0, i);
        }
        g_Data.Device.SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
    }

    bool Renderer::HasPipelineStatistics()
    {
        return g_Data.HasPipelineStatistics;
    }

    GPU::Texture Renderer::GenerateIrradianceMap(GPU::Texture const& texture)
    {
        return g_Data.IrradianceGenerator.Generate(texture);
    }
}
