#pragma once
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/PbrMaterial.h>
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class Renderer {
    public:
        static void Initialize(Window const& window);
        static void Shutdown();

        static auto BeginRendering() -> Maybe<GPU::TextureView>;
        static void EndRendering(GPU::CommandBuffer cmd);
        static void Resize(Vector2 const& new_size);

        [[nodiscard]]
        static auto Device() -> GPU::Device&;

        [[nodiscard]]
        static auto Surface() -> GPU::Surface&;

        [[nodiscard]]
        static auto GPUInstance() -> GPU::Instance&;

        [[nodiscard]]
        static auto DefaultMaterial() -> AssetRef<PbrMaterial>;

        [[nodiscard]]
        static auto DefaultNormalMap() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto WhiteTexture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto BlackTexture() -> AssetRef<Texture2D>;

        [[nodiscard]]
        static auto WhiteCubeTexture() -> GPU::Texture;

        static bool HasPipelineStatistics();

        [[nodiscard]]
        static GPU::Texture GenerateIrradianceMap(GPU::Texture const& texture);

        static void CreateDefaultResources();
    };
}
