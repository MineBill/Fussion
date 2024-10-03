#include "FussionPCH.h"
#include "ReflectionRegistry.h"

#include "Fussion/Assets/Asset.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Scene/Entity.h"
#include "Fussion/Scene/Scene.h"

namespace Fussion {
    ReflectionRegistry::ReflectionRegistry()
    {
        Register();
        RegisterGenerated();
    }

    void ReflectionRegistry::Register()
    {
        using namespace std::literals;
        using namespace Fussion;
        namespace meta = meta_hpp;
        using meta::metadata_;

        using meta::constructor_policy::as_raw_pointer;
        using meta::member_policy::as_pointer;

        // Math
        {
            meta::class_<Color>(metadata_()("Name"s, "Color"s))
                .member_("R", &Color::r, as_pointer)
                .member_("G", &Color::g, as_pointer)
                .member_("B", &Color::b, as_pointer)
                .member_("A", &Color::a, as_pointer)
                .method_("Darken", &Color::Darken)
                .method_("Lighten", &Color::Lighten);

            meta::class_<Vector2>(
                metadata_()("Name"s, "Vector2"s))
                .constructor_<f32, f32>()
                .constructor_<f64, f64>()
                .member_("X", &Vector2::x, as_pointer)
                .member_("Y", &Vector2::y, as_pointer)
                .method_("Length", &Vector2::Length)
                .method_("DistanceTo", &Vector2::DistanceTo)
                .method_("LengthSquared", &Vector2::LengthSquared)
                .method_("DistanceToSquared", &Vector2::DistanceToSquared);

            meta::class_<Vector3>(
                metadata_()("Name"s, "Vector3"s))
                .constructor_<f32, f32, f32>()
                .constructor_<f64, f64, f64>()
                .member_("X"s, &Vector3::x, as_pointer)
                .member_("Y"s, &Vector3::y, as_pointer)
                .member_("Z"s, &Vector3::z, as_pointer)
                .method_("Length"s, &Vector3::Length)
                .method_("Normalize"s, &Vector3::Normalize)
                .method_("Normalized"s, &Vector3::Normalized)
                .method_("LengthSquared"s, &Vector3::LengthSquared);
        }

        meta::class_<Asset>(metadata_()("Name"s, "Asset"s))
            .method_("GetType"s, &Asset::Type)
            .method_("GetHandle"s, &Asset::GetHandle)
            .member_("m_Handle"s, &Asset::m_Handle);

        meta::class_<Entity>(metadata_()("Name"s, "Entity"s))
            .member_("m_Parent", &Entity::m_Parent)
            .member_("m_Handle", &Entity::m_Handle)
            .member_("m_Enabled", &Entity::m_Enabled);

        meta::class_<Component>();

        meta::class_<Scene>(metadata_()("Name"s, "Scene"s))
            .method_("CreateEntity"s, &Scene::CreateEntity);

        meta::class_<AssetRefBase>(metadata_()("Name"s, "AssetRefBase"s))
            .method_("Handle"s, &AssetRefBase::GetHandle)
            .method_("SetHandle"s, &AssetRefBase::SetHandle)
            .member_("m_Handle"s, &AssetRefBase::m_Handle)
            .method_("GetType", &AssetRefBase::GetType);

        meta::class_<PbrMaterial>(metadata_()("Name"s, "PbrMaterial"s))
            .constructor_<>()
            .function_("GetStaticType", &PbrMaterial::StaticType)
            .member_("ObjectColor", &PbrMaterial::object_color, as_pointer)
            .member_("Metallic", &PbrMaterial::metallic, as_pointer)
            .member_("Roughness", &PbrMaterial::roughness, as_pointer);

        REGISTER_ENUM(GPU::TextureFormat, TextureFormat);

        meta::class_<AssetMetadata>(metadata_()("Name"s, "AssetMetadata"s));

        meta::class_<Texture2DMetadata>(metadata_()("Name"s, "Texture2DMetadata"s))
            .constructor_<>(as_raw_pointer)
            .member_("IsNormalMap", &Texture2DMetadata::IsNormalMap, as_pointer)
            .member_("Format", &Texture2DMetadata::Format, as_pointer);
    }
}

static Fussion::ReflectionRegistry const g_Registrar;
