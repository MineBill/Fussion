#pragma once
#include "AssetRef.h"
#include "Texture2D.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Rendering/UniformBuffer.h"

namespace Fussion {
    class PbrMaterial final : public Asset {
    public:
        PbrMaterial();

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

        static AssetType static_type() { return AssetType::PbrMaterial; }
        virtual AssetType type() const override { return static_type(); }

        Color object_color{};
        f32 metallic{};
        f32 roughness{};

        AssetRef<Texture2D> albedo_map{};
        AssetRef<Texture2D> normal_map{};
        AssetRef<Texture2D> ambient_occlusion_map{};
        AssetRef<Texture2D> metallic_roughness_map{};
        AssetRef<Texture2D> emissive_map{};

        struct MaterialBlock {
            Color object_color;
            f32 metallic;
            f32 roughness;
            f32 __padding1;
            f32 __padding2;
        };

        UniformBuffer<MaterialBlock> material_uniform_buffer;
    };
}
