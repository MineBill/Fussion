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
        texture->m_image_pixels.resize(data.size_in_bytes());
        Mem::copy(Span<u8>(texture->m_image_pixels), data);

        texture->m_metadata = metadata;
        GPU::TextureSpec spec {
            .label = "Texture2D Texture"sv,
            .usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .dimension = GPU::TextureDimension::D2,
            .size = { metadata.width, metadata.height, 1 },
            .format = metadata.format,
            .sample_count = 1,
            .generate_mip_maps = metadata.generate_mipmaps,
        };

        // if (metadata.is_normal_map) {
        //     spec.format = GPU::TextureFormat::RGBA8Unorm;
        // }

        auto& device = Renderer::device();

        texture->m_texture = device.create_texture(spec);

        device.write_texture(texture->m_texture, data.data(), data.size_in_bytes(), Vector2::Zero, { metadata.width, metadata.height });

        texture->m_texture.generate_mipmaps(device);

        return texture;
    }

    Ref<Texture2D> Texture2D::create(ReadOnlySpan<f32> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = make_ref<Texture2D>();
        texture->m_image_pixels.resize(data.size_in_bytes());
        Mem::copy(Span<u8>(texture->m_image_pixels), data);

        texture->m_metadata = metadata;
        GPU::TextureSpec spec {
            .label = "Texture2D Texture"sv,
            .usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .dimension = GPU::TextureDimension::D2,
            .size = { metadata.width, metadata.height, 1 },
            .format = metadata.format,
            .sample_count = 1,
            .generate_mip_maps = metadata.generate_mipmaps,
        };

        auto& device = Renderer::device();

        texture->m_texture = device.create_texture(spec);

        device.write_texture(texture->m_texture, data.data(), data.size_in_bytes(), Vector2::Zero, { metadata.width, metadata.height }, 4 * sizeof(f32));

        texture->m_texture.generate_mipmaps(device);

        return texture;
    }

    struct Texture2DHeader {
        u32 version = 1;
        u32 width, height;
        GPU::TextureFormat format;
        // RHI::FilterMode Filter{ RHI::FilterMode::Linear };
        // RHI::ImageFormat Format{ RHI::ImageFormat::R8G8B8A8_UNORM };
        // RHI::WrapMode Wrap{ RHI::WrapMode::Repeat };
        bool is_normal_map;
        bool generate_mipmaps;

        u32 image_data_size;
    };

    void Texture2D::serialize(std::ostream& stream) const
    {
        Texture2DHeader header {};
        header.width = m_metadata.width;
        header.height = m_metadata.height;
        header.format = m_metadata.format;
        header.generate_mipmaps = m_metadata.generate_mipmaps;
        header.is_normal_map = m_metadata.is_normal_map;
        header.image_data_size = cast<u32>(m_image_pixels.size());
        stream.write(reinterpret_cast<char const*>(&header), sizeof(Texture2DHeader));
        stream.write(reinterpret_cast<char const*>(m_image_pixels.data()), m_image_pixels.size() * sizeof(u8));
    }

    void Texture2D::deserialize(std::istream& stream)
    {
        Texture2DHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(Texture2DHeader));
        m_image_pixels.resize(header.image_data_size);
        stream.read(reinterpret_cast<char*>(m_image_pixels.data()), header.image_data_size * sizeof(u8));

        m_metadata.width = header.width;
        m_metadata.height = header.height;
        m_metadata.format = header.format;
        m_metadata.generate_mipmaps = header.generate_mipmaps;
        m_metadata.is_normal_map = header.is_normal_map;
        GPU::TextureSpec spec {
            .label = "Texture2D Texture"sv,
            .usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .dimension = GPU::TextureDimension::D2,
            .size = { header.width, header.height, 1 },
            .format = header.format,
            .sample_count = 1,
            .generate_mip_maps = header.generate_mipmaps,
        };

        auto& device = Renderer::device();

        m_texture = device.create_texture(spec);

        device.write_texture(
            m_texture,
            m_image_pixels.data(),
            m_image_pixels.size() * sizeof(u8),
            Vector2::Zero,
            { header.width, header.height },
            GPU::is_hdr(header.format) ? 4 * sizeof(f32) : 4
        );

        m_texture.generate_mipmaps(device);
    }
}
