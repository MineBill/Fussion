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
    std::print("Registering meta info\n");

    meta::class_<Color>(meta::metadata_()
        ("Name"s, "Color"s))
        .member_("R", &Color::R)
        .member_("G", &Color::G)
        .member_("B", &Color::B)
        .member_("A", &Color::A)
    ;

    meta::enum_<AssetType>()
        .evalue_("Invalid", AssetType::Invalid)
        .evalue_("Image", AssetType::Image)
        .evalue_("Script", AssetType::Script)
        .evalue_("Mesh", AssetType::Mesh)
        .evalue_("PbrMaterial", AssetType::PbrMaterial)
        .evalue_("Scene", AssetType::Scene)
        .evalue_("Shader", AssetType::Shader)
        .evalue_("Texture", AssetType::Texture)
        .evalue_("Texture2D", AssetType::Texture2D)
        .evalue_("HDRTexture", AssetType::HDRTexture);

    meta::class_<Asset>(meta::metadata_()
            ("Name"s, "Asset"s))
        .constructor_<>()
        .method_("GetType"s, &Asset::GetType)
        .method_("GetHandle"s, &Asset::GetHandle)
        .member_("m_Handle"s, &Asset::m_Handle)
        .member_("m_Type"s, &Asset::m_Type);

    meta::class_<Entity>(meta::metadata_()
        ("Name"s, "Entity"s));

    meta::class_<Component>();

    // region Components

    using meta::constructor_policy::as_raw_pointer;
    using meta::member_policy::as_pointer;

    meta::class_<PointLight>(meta::metadata_()
            ("Name"s, "PointLight"s))
        .constructor_<>(as_raw_pointer)
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

    meta::static_scope_("Components")
        .typedef_<MeshRenderer>("MeshRenderer"s)
        .typedef_<ScriptComponent>("ScriptComponent"s)
        .typedef_<Camera>("Camera"s)
        .typedef_<MoverComponent>("MoverComponent"s)
        .typedef_<PointLight>("PointLight"s);

    // endregion Components

    meta::class_<AssetRefBase>(meta::metadata_()
            ("Name"s, "AssetRefBase"s))
        .method_("Handle"s, &AssetRefBase::Handle)
        .method_("SetHandle"s, &AssetRefBase::SetHandle)
        .member_("m_Handle"s, &AssetRefBase::m_Handle)
        .method_("GetType", &AssetRefBase::GetType);
}

}

static const Fussion::ReflRegistrar g_Registrar;
