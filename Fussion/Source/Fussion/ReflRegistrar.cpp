#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Components/Camera.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"
#include "Fussion/Scene/Entity.h"
#include "Math/Color.h"

#include <print>
#include "ReflRegistrar.h"

#include "Scene/Components/DirectionalLight.h"

namespace Fussion {
ReflRegistrar::ReflRegistrar()
{
    Register();
}

void ReflRegistrar::Register()
{
    using namespace std::literals;
    using namespace Fussion;
    namespace meta = meta_hpp;

    using meta::constructor_policy::as_raw_pointer;
    using meta::member_policy::as_pointer;

    std::print("Registering meta info\n");

    // Math
    {
        meta::class_<Color>(meta::metadata_()
                ("Name"s, "Color"s))
            .member_("R", &Color::R, as_pointer)
            .member_("G", &Color::G, as_pointer)
            .member_("B", &Color::B, as_pointer)
            .member_("A", &Color::A, as_pointer)
            .method_("Darken", &Color::Darken)
            .method_("Lighten", &Color::Lighten);

        meta::class_<Vector2>(
                meta::metadata_()("Name"s, "Vector2"s))
            .constructor_<f32, f32>()
            .constructor_<f64, f64>()
            .member_("X", &Vector2::X, as_pointer)
            .member_("Y", &Vector2::Y, as_pointer)
            .method_("Length", &Vector2::Length)
            .method_("DistanceTo", &Vector2::DistanceTo)
            .method_("LengthSquared", &Vector2::LengthSquared)
            .method_("DistanceToSquared", &Vector2::DistanceToSquared);

        meta::class_<Vector3>(
                meta::metadata_()("Name"s, "Vector3"s))
            .constructor_<f32, f32, f32>()
            .constructor_<f64, f64, f64>()
            .member_("X"s, &Vector3::X, as_pointer)
            .member_("Y"s, &Vector3::Y, as_pointer)
            .member_("Z"s, &Vector3::Z, as_pointer)
            .method_("Length"s, &Vector3::Length)
            .method_("Normalize"s, &Vector3::Normalize)
            .method_("Normalized"s, &Vector3::Normalized)
            .method_("LengthSquared"s, &Vector3::LengthSquared);
    }

    meta::enum_<AssetType>()
        .evalue_("Invalid"s, AssetType::Invalid)
        .evalue_("Image"s, AssetType::Image)
        .evalue_("Script"s, AssetType::Script)
        .evalue_("Mesh"s, AssetType::Mesh)
        .evalue_("PbrMaterial"s, AssetType::PbrMaterial)
        .evalue_("Scene"s, AssetType::Scene)
        .evalue_("Shader"s, AssetType::Shader)
        .evalue_("Texture"s, AssetType::Texture)
        .evalue_("Texture2D"s, AssetType::Texture2D)
        .evalue_("HDRTexture"s, AssetType::HDRTexture);

    meta::class_<Asset>(meta::metadata_()
            ("Name"s, "Asset"s))
        .method_("GetType"s, &Asset::GetType)
        .method_("GetHandle"s, &Asset::GetHandle)
        .member_("m_Handle"s, &Asset::m_Handle);

    meta::class_<Entity>(meta::metadata_()
            ("Name"s, "Entity"s))
        .member_("m_Parent", &Entity::m_Parent)
        .member_("m_Handle", &Entity::m_Handle)
        .member_("m_Enabled", &Entity::m_Enabled);

    // region Components
    {
        meta::class_<Component>();

        meta::class_<PointLight>(meta::metadata_()
                ("Name"s, "PointLight"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer)
            .member_("Offset", &PointLight::Offset, as_pointer)
            .member_("Radius", &PointLight::Radius, as_pointer);

        meta::class_<DirectionalLight>(meta::metadata_()
                ("Name"s, "DirectionalLight"s))
            .constructor_<Entity*>(as_raw_pointer);

        meta::class_<Camera>(meta::metadata_()
                ("Name"s, "Camera"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer)
            .member_("FieldOfView"s, &Camera::FieldOfView, as_pointer, meta::metadata_()
                ("Range"s, "1, 100"s)
                )
            .member_("SignedType"s, &Camera::SignedType, as_pointer)
            .member_("UnsignedType"s, &Camera::UnsignedType, as_pointer)
            .member_("AStringToo"s, &Camera::AStringToo, as_pointer);

        meta::class_<MeshRenderer>(meta::metadata_()
                ("Name"s, "MeshRenderer"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer)
            .member_("Mesh"s, &MeshRenderer::Mesh, as_pointer)
            .member_("Material"s, &MeshRenderer::Material, as_pointer);

        meta::class_<ScriptComponent>(meta::metadata_()
                ("Name"s, "ScriptComponent"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer);

        meta::class_<MoverComponent>(meta::metadata_()
                ("Name"s, "MoverComponent"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer)
            .member_("Speed"s, &MoverComponent::Speed, as_pointer);

        meta::enum_<DebugDrawer::Type>()
            .evalue_("Box"s, DebugDrawer::Type::Box)
            .evalue_("Sphere"s, DebugDrawer::Type::Sphere);

        meta::class_<DebugDrawer>(meta::metadata_()
                ("Name"s, "DebugDrawer"s))
            .constructor_<>(as_raw_pointer)
            .constructor_<Entity*>(as_raw_pointer)
            .member_("DrawType"s, &DebugDrawer::DrawType, as_pointer)
            .member_("Size"s, &DebugDrawer::Size, as_pointer);

        meta::static_scope_("Components")
            .typedef_<MeshRenderer>("MeshRenderer"s)
            .typedef_<ScriptComponent>("ScriptComponent"s)
            .typedef_<Camera>("Camera"s)
            .typedef_<MoverComponent>("MoverComponent"s)
            .typedef_<PointLight>("PointLight"s)
            .typedef_<DebugDrawer>("DebugDrawer"s)
            .typedef_<DirectionalLight>("DirectionalLight"s);

    }
    // endregion Components

    meta::class_<AssetRefBase>(meta::metadata_()
            ("Name"s, "AssetRefBase"s))
        .method_("Handle"s, &AssetRefBase::Handle)
        .method_("SetHandle"s, &AssetRefBase::SetHandle)
        .member_("m_Handle"s, &AssetRefBase::m_Handle)
        .method_("GetType", &AssetRefBase::GetType);

    meta::class_<PbrMaterial>(meta::metadata_()
            ("Name"s, "PbrMaterial"s))
        .constructor_<>()
        .function_("GetStaticType", &PbrMaterial::GetStaticType)
        .member_("ObjectColor", &PbrMaterial::ObjectColor, as_pointer);
}

}

static const Fussion::ReflRegistrar g_Registrar;
