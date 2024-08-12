#pragma once
#include "Asset.h"
#include "Fussion/RHI/Image.h"

namespace Fussion {

    class Texture2DMetadata final : public AssetMetadata {
        META_HPP_ENABLE_POLY_INFO(AssetMetadata)
    public:
        s32 Width{}, Height{};
        RHI::FilterMode Filter = RHI::FilterMode::Linear;
        RHI::ImageFormat Format = RHI::ImageFormat::R8G8B8A8_SRGB;
        RHI::WrapMode Wrap = RHI::WrapMode::Repeat;
        bool IsNormalMap{ false };

        f32 Aspect() const { return CAST(f32, Width) / CAST(f32, Height); }
    };

    class Texture2D final : public Asset {
    public:
        static Ref<Texture2D> Create(std::span<u8> data, Texture2DMetadata const& metadata);

        Ref<RHI::Image>& GetImage() { return m_Image; }

        Texture2DMetadata const& Metadata() const { return m_Metadata; }

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        AssetType GetType() const override { return GetStaticType(); }

    private:
        Ref<RHI::Image> m_Image{};
        Texture2DMetadata m_Metadata{};
    };
}
