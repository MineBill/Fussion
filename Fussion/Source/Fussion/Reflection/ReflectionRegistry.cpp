#include "FussionPCH.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Scene/Entity.h"
#include "Math/Color.h"

#include "ReflectionRegistry.h"

#include "Assets/AssetRef.h"
#include "Assets/PbrMaterial.h"
#include "Input/Input.h"
#include "Scene/Scene.h"

namespace Fussion {
    void register_generated();

    ReflectionRegistry::ReflectionRegistry()
    {
        Register();
        register_generated();
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
            meta::class_<Color>(metadata_()
                    ("Name"s, "Color"s))
                .member_("R", &Color::r, as_pointer)
                .member_("G", &Color::g, as_pointer)
                .member_("B", &Color::b, as_pointer)
                .member_("A", &Color::a, as_pointer)
                .method_("Darken", &Color::darken)
                .method_("Lighten", &Color::lighten);

            meta::class_<Vector2>(
                    metadata_()
                    ("Name"s, "Vector2"s))
                .constructor_<f32, f32>()
                .constructor_<f64, f64>()
                .member_("X", &Vector2::x, as_pointer)
                .member_("Y", &Vector2::y, as_pointer)
                .method_("Length", &Vector2::length)
                .method_("DistanceTo", &Vector2::distance_to)
                .method_("LengthSquared", &Vector2::length_squared)
                .method_("DistanceToSquared", &Vector2::distance_to_squared);

            meta::class_<Vector3>(
                    metadata_()
                    ("Name"s, "Vector3"s))
                .constructor_<f32, f32, f32>()
                .constructor_<f64, f64, f64>()
                .member_("X"s, &Vector3::x, as_pointer)
                .member_("Y"s, &Vector3::y, as_pointer)
                .member_("Z"s, &Vector3::z, as_pointer)
                .method_("Length"s, &Vector3::length)
                .method_("Normalize"s, &Vector3::normalize)
                .method_("Normalized"s, &Vector3::normalized)
                .method_("LengthSquared"s, &Vector3::length_squared);
        }

        meta::class_<Asset>(metadata_()
                ("Name"s, "Asset"s))
            .method_("GetType"s, &Asset::type)
            .method_("GetHandle"s, &Asset::handle)
            .member_("m_Handle"s, &Asset::m_handle);

        meta::class_<Entity>(metadata_()
                ("Name"s, "Entity"s))
            .member_("m_Parent", &Entity::m_parent)
            .member_("m_Handle", &Entity::m_handle)
            .member_("m_Enabled", &Entity::m_enabled);

        meta::class_<Component>();

        meta::class_<Scene>(metadata_()
                ("Name"s, "Scene"s))
            .method_("CreateEntity"s, &Scene::create_entity);

        meta::class_<AssetRefBase>(metadata_()
                ("Name"s, "AssetRefBase"s))
            .method_("Handle"s, &AssetRefBase::handle)
            .method_("SetHandle"s, &AssetRefBase::set_handle)
            .member_("m_Handle"s, &AssetRefBase::m_handle)
            .method_("GetType", &AssetRefBase::type);

        meta::class_<PbrMaterial>(metadata_()
                ("Name"s, "PbrMaterial"s))
            .constructor_<>()
            .function_("GetStaticType", &PbrMaterial::static_type)
            .member_("ObjectColor", &PbrMaterial::object_color, as_pointer)
            .member_("Metallic", &PbrMaterial::metallic, as_pointer)
            .member_("Roughness", &PbrMaterial::roughness, as_pointer);

        REGISTER_ENUM(GPU::TextureFormat, TextureFormat);

        meta::class_<AssetMetadata>(metadata_()("Name"s, "AssetMetadata"s));

        meta::class_<Texture2DMetadata>(metadata_()("Name"s, "Texture2DMetadata"s))
            .constructor_<>(as_raw_pointer)
            .member_("IsNormalMap", &Texture2DMetadata::is_normal_map, as_pointer)
            // .member_("Filter", &Texture2DMetadata::Filter, as_pointer)
            // .member_("Wrap", &Texture2DMetadata::Wrap, as_pointer)
            .member_("Format", &Texture2DMetadata::format, as_pointer);
    }
}

static const Fussion::ReflectionRegistry g_Registrar;
