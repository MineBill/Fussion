#include "FussionPCH.h"
#include "PbrMaterial.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

Fussion::PbrMaterial::PbrMaterial()
{
    material_uniform_buffer = UniformBuffer<MaterialBlock>::create(Renderer::device(), "Material"sv);
}

void Fussion::PbrMaterial::serialize(Serializer& ctx) const
{
    FSN_SERIALIZE_MEMBER(metallic);
    FSN_SERIALIZE_MEMBER(roughness);
    FSN_SERIALIZE_MEMBER(object_color);

    FSN_SERIALIZE_MEMBER(albedo_map);
    FSN_SERIALIZE_MEMBER(emissive_map);
    FSN_SERIALIZE_MEMBER(normal_map);
    FSN_SERIALIZE_MEMBER(metallic_roughness_map);
    FSN_SERIALIZE_MEMBER(ambient_occlusion_map);
}

void Fussion::PbrMaterial::deserialize(Deserializer& ctx)
{
    FSN_DESERIALIZE_MEMBER(metallic);
    FSN_DESERIALIZE_MEMBER(roughness);
    FSN_DESERIALIZE_MEMBER(object_color);

    FSN_DESERIALIZE_MEMBER(albedo_map);
    FSN_DESERIALIZE_MEMBER(emissive_map);
    FSN_DESERIALIZE_MEMBER(normal_map);
    FSN_DESERIALIZE_MEMBER(metallic_roughness_map);
    FSN_DESERIALIZE_MEMBER(ambient_occlusion_map);
}
