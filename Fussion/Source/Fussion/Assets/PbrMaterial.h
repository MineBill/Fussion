#pragma once
#include "AssetRef.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Rendering/UniformBuffer.h"
#include "Texture2D.h"

namespace Fussion {
    class PbrMaterial final : public Asset {
    public:
        PbrMaterial();

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

        static AssetType StaticType() { return AssetType::PbrMaterial; }
        virtual AssetType Type() const override { return StaticType(); }

        void UpdateBuffer();
        void UpdateSampler();

        Color object_color {};
        f32 metallic {};
        f32 roughness {};

        Vector2 tiling { 1.0f, 1.0f };
        AssetRef<Texture2D> albedo_map {};
        AssetRef<Texture2D> normal_map {};
        AssetRef<Texture2D> ambient_occlusion_map {};
        AssetRef<Texture2D> metallic_roughness_map {};
        AssetRef<Texture2D> emissive_map {};

        struct MaterialBlock {
            Color object_color;
            f32 metallic;
            f32 roughness;
            Vector2 tilling;
        };

        UniformBuffer<MaterialBlock> material_uniform_buffer;
        GPU::Sampler sampler {};
    };
}
