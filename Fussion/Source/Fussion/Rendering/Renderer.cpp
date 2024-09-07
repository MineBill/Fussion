#include "FussionPCH.h"
#include "Renderer.h"

#include "Fussion/Util/TextureImporter.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/GPU/Utils.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

using namespace Fussion::RHI;

namespace Fussion {
    Renderer* Renderer::s_Renderer = nullptr;

    void Renderer::Initialize(Window const& window)
    {
        LOG_INFO("Initializing Renderer");
        s_Renderer = new Renderer();

        // auto instance = Instance::Create(window);
        // auto* device = Device::Create(instance);
        // Device::SetInstance(device);

        auto instance = GPU::Instance::Create({
            .Backend = GPU::BackendRenderer::Vulkan,
        });

        auto surface = instance.GetSurface(&window);
        auto adapter = instance.GetAdapter(surface, {
            .PowerPreference = GPU::DevicePower::HighPerformance
        });

        auto device = adapter.GetDevice();

        s_Renderer->m_WindowSize = window.GetSize();

        surface.Configure(device, adapter, {
            .PresentMode = GPU::PresentMode::Immediate,
            .Size = s_Renderer->m_WindowSize
        });

        s_Renderer->m_Device = device;
        s_Renderer->m_Adapter = adapter;
        s_Renderer->m_Instance = instance;
        s_Renderer->m_Surface = surface;

        // NOTE: Setting the callback on m_Device, rather than the device local variable, because
        //       the GPU::Device will set up WGPU user data with its pointer.
        s_Renderer->m_Device.SetErrorCallback([&](GPU::ErrorType type, std::string_view message) {
            LOG_ERRORF("!DEVICE ERROR!\n\tTYPE: {}\n\tMESSAGE: {}", magic_enum::enum_name(type), message);
        });

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

    auto Renderer::Begin() -> Maybe<GPU::TextureView>
    {
        ZoneScoped;
        auto new_size = Application::Instance()->GetWindow().GetSize();
        if (s_Renderer->m_WindowSize != new_size) {
            Resize(new_size);
        }

        if (s_Renderer->m_SkipRender)
            return None();

        auto view = s_Renderer->m_Surface.GetNextView();
        if (view.IsError()) {
            // Handle resize or the reason the view is empty
            // ...
            return None();
        }
        return view.Value();
    }

    void Renderer::End(GPU::CommandBuffer cmd)
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
        s_Renderer->m_Surface.Configure(s_Renderer->m_Device, s_Renderer->m_Adapter, {
            .PresentMode = GPU::PresentMode::Immediate,
            .Size = new_size
        });
    }

    auto Renderer::DefaultNormalMap() -> AssetRef<Texture2D> { return s_Renderer->m_NormalMap; }

    auto Renderer::WhiteTexture() -> AssetRef<Texture2D> { return s_Renderer->m_WhiteTexture; }

    auto Renderer::BlackTexture() -> AssetRef<Texture2D> { return s_Renderer->m_BlackTexture; }

    static unsigned char g_white_texture_png[] = {
#include "white_texture.png.h"
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
        material->ObjectColor = Color(1, 1, 1, 1);
        s_Renderer->m_DefaultMaterial = AssetManager::CreateVirtualAssetRef<PbrMaterial>(material);

        s_Renderer->m_WhiteTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_white_texture_png).Value(), "Default White Texture");
        s_Renderer->m_BlackTexture = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_black_texture_png).Value(), "Default Black Texture");
        s_Renderer->m_NormalMap = AssetManager::CreateVirtualAssetRef<Texture2D>(TextureImporter::LoadTextureFromMemory(g_normal_map_png, true).Value(), "Default Normal Map");

    }
}
