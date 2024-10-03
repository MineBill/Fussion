#pragma once
#include "Asset.h"
#include "Fussion/GPU/GPU.h"

namespace Fussion {
    class Texture2DMetadata final : public AssetMetadata {
        META_HPP_ENABLE_POLY_INFO(AssetMetadata)
    public:
        u32 Width{}, Height{};
        GPU::TextureFormat Format{ GPU::TextureFormat::RGBA8UnormSrgb };
        // RHI::FilterMode Filter{ RHI::FilterMode::Linear };
        // RHI::ImageFormat Format{ RHI::ImageFormat::R8G8B8A8_UNORM };
        // RHI::WrapMode Wrap{ RHI::WrapMode::Repeat };
        bool IsNormalMap{ false };
        bool GenerateMipmaps{ true };

        [[nodiscard]]
        f32 Aspect() const { return CAST(f32, Width) / CAST(f32, Height); }

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class Texture2D final : public Asset {
    public:
        static Ref<Texture2D> Create(std::span<u8> data, Texture2DMetadata const& metadata);
        static Ref<Texture2D> Create(std::span<f32> data, Texture2DMetadata const& metadata);

        GPU::Texture& GetTexture() { return m_Texture; }

        Texture2DMetadata const& GetMetadata() const { return m_Metadata; }

        static AssetType StaticType() { return AssetType::Texture2D; }
        virtual AssetType Type() const override { return StaticType(); }

    private:
        GPU::Texture m_Texture{};
        Texture2DMetadata m_Metadata{};
    };
}
