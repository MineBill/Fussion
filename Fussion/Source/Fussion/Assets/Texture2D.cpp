﻿#include "FussionPCH.h"

#include "Texture2D.h"

#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void Texture2DMetadata::Serialize(Serializer& ctx) const
    {
        AssetMetadata::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Width);
        FSN_SERIALIZE_MEMBER(Height);
        // FSN_SERIALIZE_MEMBER(Filter);
        FSN_SERIALIZE_MEMBER(Format);
        // FSN_SERIALIZE_MEMBER(Wrap);
        FSN_SERIALIZE_MEMBER(IsNormalMap);
        FSN_SERIALIZE_MEMBER(GenerateMipmaps);
    }

    void Texture2DMetadata::Deserialize(Deserializer& ctx)
    {
        AssetMetadata::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Width);
        FSN_DESERIALIZE_MEMBER(Height);
        // FSN_DESERIALIZE_MEMBER(Filter);
        FSN_DESERIALIZE_MEMBER(Format);
        // FSN_DESERIALIZE_MEMBER(Wrap);
        FSN_DESERIALIZE_MEMBER(IsNormalMap);
        FSN_DESERIALIZE_MEMBER(GenerateMipmaps);
    }

    Ref<Texture2D> Texture2D::Create(std::span<u8> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = MakeRef<Texture2D>();

        texture->m_Metadata = metadata;
        GPU::TextureSpec spec {
            .Label = "Texture2D Texture"sv,
            .Usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { metadata.Width, metadata.Height, 1 },
            .Format = metadata.Format,
            .SampleCount = 1,
            .GenerateMipMaps = metadata.GenerateMipmaps,
        };

        if (metadata.IsNormalMap) {
            spec.Format = GPU::TextureFormat::RGBA8Unorm;
        }

        auto& device = Renderer::Device();

        texture->m_Texture = device.CreateTexture(spec);

        device.WriteTexture(texture->m_Texture, data.data(), data.size_bytes(), Vector2::Zero, { metadata.Width, metadata.Height });

        texture->m_Texture.GenerateMipmaps(device);

        return texture;
    }

    Ref<Texture2D> Texture2D::Create(std::span<f32> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = MakeRef<Texture2D>();

        texture->m_Metadata = metadata;
        GPU::TextureSpec spec {
            .Label = "Texture2D Texture"sv,
            .Usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { metadata.Width, metadata.Height, 1 },
            .Format = metadata.Format,
            .SampleCount = 1,
            .GenerateMipMaps = metadata.GenerateMipmaps,
        };

        auto& device = Renderer::Device();

        texture->m_Texture = device.CreateTexture(spec);

        device.WriteTexture(texture->m_Texture, data.data(), data.size_bytes(), Vector2::Zero, { metadata.Width, metadata.Height }, 4 * sizeof(f32));

        texture->m_Texture.GenerateMipmaps(device);

        return texture;
    }
}
