#include "FussionPCH.h"
#include "Texture2D.h"

#include "AssetManager.h"
#include "RHI/Device.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"
#include "GPU/Utils.h"

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

    Ref<Texture2D> Texture2D::create(std::span<u8> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = make_ref<Texture2D>();

        texture->m_metadata = metadata;
        GPU::TextureSpec spec{
            .label = "Texture2D Texture"sv,
            .usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding | GPU::TextureUsage::CopySrc,
            .dimension = GPU::TextureDimension::D2,
            .size = { metadata.width, metadata.height, 1 },
            .format = metadata.format,
            .sample_count = 1,
            .generate_mip_maps = metadata.generate_mipmaps,
        };

        auto& device = Renderer::device();

        texture->m_image = device.create_texture(spec);

        device.write_texture(texture->m_image, data.data(), data.size_bytes(), Vector2::Zero, { metadata.width, metadata.height });

        texture->m_image.generate_mipmaps(device);

        return texture;
    }
}
