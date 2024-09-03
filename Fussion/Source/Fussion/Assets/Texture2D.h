#pragma once
#include "Asset.h"
#include "Fussion/GPU/GPU.h"

namespace Fussion {
    class Texture2DMetadata final : public AssetMetadata {
        META_HPP_ENABLE_POLY_INFO(AssetMetadata)
    public:
        s32 Width{}, Height{};
        GPU::TextureFormat Format{ GPU::TextureFormat::RGBA8Unorm };
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

        GPU::Texture& GetImage() { return m_Image; }

        Texture2DMetadata const& Metadata() const { return m_Metadata; }

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        virtual AssetType GetType() const override { return GetStaticType(); }

    private:
        GPU::Texture m_Image{};
        Texture2DMetadata m_Metadata{};
    };
}
