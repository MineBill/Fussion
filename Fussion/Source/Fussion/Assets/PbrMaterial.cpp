#include "FussionPCH.h"
#include "PbrMaterial.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

Fussion::PbrMaterial::PbrMaterial()
{
    using namespace std::string_view_literals;
    MaterialUniformBuffer = UniformBuffer<MaterialBlock>::Create(Renderer::Device(), "Material"sv);
}

void Fussion::PbrMaterial::Serialize(Serializer& ctx) const
{
    FSN_SERIALIZE_MEMBER(Metallic);
    FSN_SERIALIZE_MEMBER(Roughness);
    FSN_SERIALIZE_MEMBER(ObjectColor);

    FSN_SERIALIZE_MEMBER(AlbedoMap);
    FSN_SERIALIZE_MEMBER(EmissiveMap);
    FSN_SERIALIZE_MEMBER(NormalMap);
    FSN_SERIALIZE_MEMBER(MetallicRoughnessMap);
    FSN_SERIALIZE_MEMBER(AmbientOcclusionMap);
}

void Fussion::PbrMaterial::Deserialize(Deserializer& ctx)
{
    FSN_DESERIALIZE_MEMBER(Metallic);
    FSN_DESERIALIZE_MEMBER(Roughness);
    FSN_DESERIALIZE_MEMBER(ObjectColor);

    FSN_DESERIALIZE_MEMBER(AlbedoMap);
    FSN_DESERIALIZE_MEMBER(EmissiveMap);
    FSN_DESERIALIZE_MEMBER(NormalMap);
    FSN_DESERIALIZE_MEMBER(MetallicRoughnessMap);
    FSN_DESERIALIZE_MEMBER(AmbientOcclusionMap);
}
