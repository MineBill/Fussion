#include "PbrMaterial.h"

#include "FussionPCH.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

Fussion::PbrMaterial::PbrMaterial()
{
    material_uniform_buffer = UniformBuffer<MaterialBlock>::Create(Renderer::Device(), "Material"sv);
    UpdateSampler();
}

void Fussion::PbrMaterial::Serialize(Serializer& ctx) const
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

void Fussion::PbrMaterial::Deserialize(Deserializer& ctx)
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

void Fussion::PbrMaterial::UpdateBuffer()
{
    material_uniform_buffer.Data.metallic = metallic;
    material_uniform_buffer.Data.roughness = roughness;
    material_uniform_buffer.Data.object_color = object_color;
    material_uniform_buffer.Data.tilling = tiling;
    material_uniform_buffer.flush();
}

void Fussion::PbrMaterial::UpdateSampler()
{
    sampler.Release();
    GPU::SamplerSpec bilinear_sampler_spec {
        .label = "Material Sampler"sv,
        .AddressModeU = GPU::AddressMode::Repeat,
        .AddressModeV = GPU::AddressMode::Repeat,
        .AddressModeW = GPU::AddressMode::Repeat,
        .MagFilter = GPU::FilterMode::Linear,
        .MinFilter = GPU::FilterMode::Linear,
        .MipMapFilter = GPU::FilterMode::Linear,
        .LodMinClamp = 0.f,
        .LodMaxClamp = 32.f,
        .AnisotropyClamp = 16
    };

    sampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);
}
