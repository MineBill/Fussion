#include "FussionPCH.h"

#include "Texture2D.h"

#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void Texture2DMetadata::serialize(Serializer& ctx) const
    {
        AssetMetadata::serialize(ctx);
        FSN_SERIALIZE_MEMBER(width);
        FSN_SERIALIZE_MEMBER(height);
        // FSN_SERIALIZE_MEMBER(Filter);
        FSN_SERIALIZE_MEMBER(format);
        // FSN_SERIALIZE_MEMBER(Wrap);
        FSN_SERIALIZE_MEMBER(is_normal_map);
        FSN_SERIALIZE_MEMBER(generate_mipmaps);
    }

    void Texture2DMetadata::deserialize(Deserializer& ctx)
    {
        AssetMetadata::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(width);
        FSN_DESERIALIZE_MEMBER(height);
        // FSN_DESERIALIZE_MEMBER(Filter);
        FSN_DESERIALIZE_MEMBER(format);
        // FSN_DESERIALIZE_MEMBER(Wrap);
        FSN_DESERIALIZE_MEMBER(is_normal_map);
        FSN_DESERIALIZE_MEMBER(generate_mipmaps);
    }

    Ref<Texture2D> Texture2D::create(ReadOnlySpan<u8> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = make_ref<Texture2D>();

        texture->m_metadata = metadata;
        GPU::TextureSpec spec {
            .Label = "Texture2D Texture"sv,
            .Usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { metadata.width, metadata.height, 1 },
            .Format = metadata.format,
            .SampleCount = 1,
            .GenerateMipMaps = metadata.generate_mipmaps,
        };

        if (metadata.is_normal_map) {
            spec.Format = GPU::TextureFormat::RGBA8Unorm;
        }

        auto& device = Renderer::device();

        texture->m_texture = device.CreateTexture(spec);

        device.WriteTexture(texture->m_texture, data.data(), data.size_in_bytes(), Vector2::Zero, { metadata.width, metadata.height });

        texture->m_texture.GenerateMipmaps(device);

        return texture;
    }

    Ref<Texture2D> Texture2D::create(ReadOnlySpan<f32> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = make_ref<Texture2D>();

        texture->m_metadata = metadata;
        GPU::TextureSpec spec {
            .Label = "Texture2D Texture"sv,
            .Usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { metadata.width, metadata.height, 1 },
            .Format = metadata.format,
            .SampleCount = 1,
            .GenerateMipMaps = metadata.generate_mipmaps,
        };

        auto& device = Renderer::device();

        texture->m_texture = device.CreateTexture(spec);

        device.WriteTexture(texture->m_texture, data.data(), data.size_in_bytes(), Vector2::Zero, { metadata.width, metadata.height }, 4 * sizeof(f32));

        texture->m_texture.GenerateMipmaps(device);

        return texture;
    }
}
