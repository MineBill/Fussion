#pragma once
#include "Asset.h"
#include "Fussion/GPU/GPU.h"

namespace Fussion {
    class Texture2DMetadata final : public AssetMetadata {
        META_HPP_ENABLE_POLY_INFO(AssetMetadata)
    public:
        u32 width{}, height{};
        GPU::TextureFormat format{ GPU::TextureFormat::RGBA8UnormSrgb };
        // RHI::FilterMode Filter{ RHI::FilterMode::Linear };
        // RHI::ImageFormat Format{ RHI::ImageFormat::R8G8B8A8_UNORM };
        // RHI::WrapMode Wrap{ RHI::WrapMode::Repeat };
        bool is_normal_map{ false };
        bool generate_mipmaps{ true };

        [[nodiscard]]
        f32 aspect() const { return CAST(f32, width) / CAST(f32, height); }

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class Texture2D final : public Asset {
    public:
        static Ref<Texture2D> create(std::span<u8> data, Texture2DMetadata const& metadata);
        static Ref<Texture2D> create(std::span<f32> data, Texture2DMetadata const& metadata);

        GPU::Texture& image() { return m_image; }

        Texture2DMetadata const& metadata() const { return m_metadata; }

        static AssetType static_type() { return AssetType::Texture2D; }
        virtual AssetType type() const override { return static_type(); }

    private:
        GPU::Texture m_image{};
        Texture2DMetadata m_metadata{};
    };
}
