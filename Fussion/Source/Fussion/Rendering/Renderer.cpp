#include "FussionPCH.h"
#include "Renderer.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Core/Application.h"
#include "Fussion/GPU/Utils.h"
#include "Fussion/Util/TextureImporter.h"
#include "Pipelines/IrradianceIBLGenerator.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Renderer* Renderer::s_Renderer = nullptr;

    struct Data {
        IrradianceIBLGenerator IrradianceGenerator;
    } g_Data;

    void Renderer::Initialize(Window const& window)
    {
        LOG_INFO("Initializing Renderer");
        s_Renderer = new Renderer();

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
            s_Renderer->m_HasPipelineStatistics = true;
            spec.RequiredFeatures.push_back(GPU::Feature::PipelineStatistics);
        }
        auto device = adapter.RequestDevice(spec);

        s_Renderer->m_WindowSize = window.Size();

        surface.Configure(device, adapter, { .Mode = GPU::PresentMode::Fifo, .Size = s_Renderer->m_WindowSize });

        s_Renderer->m_Device = device;
        s_Renderer->m_Adapter = adapter;
        s_Renderer->m_Instance = instance;
        s_Renderer->m_Surface = surface;

        g_Data.IrradianceGenerator.Initialize();

        GPU::Utils::RenderDoc::Initialize();
    }

    void Renderer::Shutdown()
    {
        LOG_DEBUGF("Shutting down Renderer!");

        s_Renderer->m_Device.Release();
        s_Renderer->m_Adapter.Release();
        s_Renderer->m_Surface.Release();
        s_Renderer->m_Instance.Release();
    }

    auto Renderer::BeginRendering() -> Maybe<GPU::TextureView>
    {
        ZoneScoped;
        auto new_size = Application::Self()->GetWindow().Size();
        if (s_Renderer->m_WindowSize != new_size) {
            Resize(new_size);
        }

        if (s_Renderer->m_SkipRender)
            return None();

        auto view = s_Renderer->m_Surface.GetNextView();
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
        s_Renderer->m_Device.SubmitCommandBuffer(cmd);
        s_Renderer->m_Surface.Present();
        cmd.Release();
    }

    void Renderer::Resize(Vector2 const& new_size)
    {
        ZoneScoped;
        if (new_size.IsZero()) {
            s_Renderer->m_SkipRender = true;
            return;
        }
        s_Renderer->m_SkipRender = false;

        s_Renderer->m_WindowSize = new_size;
        s_Renderer->m_Surface.Configure(s_Renderer->m_Device, s_Renderer->m_Adapter, { .Mode = GPU::PresentMode::Immediate, .Size = new_size });
    }

    auto Renderer::DefaultNormalMap() -> AssetRef<Texture2D> { return s_Renderer->m_NormalMap; }

    auto Renderer::WhiteTexture() -> AssetRef<Texture2D> { return s_Renderer->m_WhiteTexture; }

    auto Renderer::BlackTexture() -> AssetRef<Texture2D> { return s_Renderer->m_BlackTexture; }

    auto Renderer::WhiteCubeTexture() -> GPU::Texture
    {
        return s_Renderer->m_WhiteCubeTexture;
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
        s_Renderer->m_DefaultMaterial = AssetManager::CreateVirtualAssetRef<PbrMaterial>(material);

        s_Renderer->m_WhiteTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_white_texture_png).Unwrap(), "Default White Texture");
        s_Renderer->m_BlackTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_black_texture_png).Unwrap(), "Default Black Texture");
        s_Renderer->m_NormalMap = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_normal_map_png, true).Unwrap(), "Default Normal Map");

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

        s_Renderer->m_WhiteCubeTexture = Device().CreateTexture(texture_spec);
        s_Renderer->m_WhiteCubeTexture.View = s_Renderer->m_WhiteCubeTexture.CreateView({
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

        auto encoder = m_Device.CreateCommandEncoder();

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(data.get()->GetTexture(), m_WhiteCubeTexture, { 1, 1 }, 0, 0, 0, i);
        }
        m_Device.SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
    }

    bool Renderer::HasPipelineStatistics()
    {
        return s_Renderer->m_HasPipelineStatistics;
    }

    GPU::Texture Renderer::GenerateIrradianceMap(GPU::Texture const& texture)
    {
        return g_Data.IrradianceGenerator.Generate(texture);
    }
}
