#include "PbrMaterial.h"

#include "FussionPCH.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

Fussion::PbrMaterial::PbrMaterial()
{
    material_uniform_buffer = UniformBuffer<MaterialBlock>::create(Renderer::device(), "Material"sv);
    update_sampler();
}

void Fussion::PbrMaterial::serialize(Serializer& ctx) const
{
    FSN_SERIALIZE_MEMBER(metallic);
    FSN_SERIALIZE_MEMBER(roughness);
    FSN_SERIALIZE_MEMBER(object_color);

    FSN_SERIALIZE_MEMBER(tiling);
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

    FSN_DESERIALIZE_MEMBER(tiling);
    FSN_DESERIALIZE_MEMBER(albedo_map);
    FSN_DESERIALIZE_MEMBER(emissive_map);
    FSN_DESERIALIZE_MEMBER(normal_map);
    FSN_DESERIALIZE_MEMBER(metallic_roughness_map);
    FSN_DESERIALIZE_MEMBER(ambient_occlusion_map);
}

void Fussion::PbrMaterial::update_buffer()
{
    material_uniform_buffer.data.metallic = metallic;
    material_uniform_buffer.data.roughness = roughness;
    material_uniform_buffer.data.object_color = object_color;
    material_uniform_buffer.data.tilling = tiling;
    material_uniform_buffer.flush();
}

void Fussion::PbrMaterial::update_sampler()
{
    sampler.release();
    GPU::SamplerSpec bilinear_sampler_spec {
        .label = "Material Sampler"sv,
        .address_mode_u = GPU::AddressMode::Repeat,
        .address_mode_v = GPU::AddressMode::Repeat,
        .address_mode_w = GPU::AddressMode::Repeat,
        .mag_filter = GPU::FilterMode::Linear,
        .min_filter = GPU::FilterMode::Linear,
        .mip_map_filter = GPU::FilterMode::Linear,
        .lod_min_clamp = 0.f,
        .lod_max_clamp = 32.f,
        .anisotropy_clamp = 16
    };

    sampler = Renderer::device().create_sampler(bilinear_sampler_spec);
}
