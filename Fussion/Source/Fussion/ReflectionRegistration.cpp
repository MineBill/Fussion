#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Components/Camera.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"
#include "Fussion/Scene/Entity.h"

#include <print>

class Empty
{

};

namespace
{
    void Register()
    {
        using namespace std::literals;
        using namespace Fussion;
        namespace meta = meta_hpp;
        std::print("Registering meta info\n");

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
            .evalue_("HDRTexture", AssetType::HDRTexture)
        ;

        meta::class_<Asset>(meta::metadata_()
            ("Name"s, "Asset"s))
            .constructor_<>()
            .method_("GetType", &Asset::GetType)
            .method_("GetHandle", &Asset::GetHandle)
        ;

        meta::class_<Entity>(meta::metadata_()
            ("Name"s, "Entity"s))
        ;

        meta::class_<Component>();

        // region Components

        meta::class_<PointLight>(meta::metadata_()
            ("Name"s, "PointLight"s))
            .constructor_<>(meta::constructor_policy::as_raw_pointer)
            .constructor_<Entity*>(meta::constructor_policy::as_raw_pointer)
        ;

        meta::class_<Camera>(meta::metadata_()
            ("Name"s, "Camera"s))
            .constructor_<>(meta::constructor_policy::as_raw_pointer)
            .constructor_<Entity*>(meta::constructor_policy::as_raw_pointer)
            .member_("FieldOfView", &Camera::FieldOfView, meta::metadata_()
                ("Range"s, "1, 100"s)
            )
        ;

        meta::class_<MeshRenderer>(meta::metadata_()
            ("Name"s, "MeshRenderer"s))
            .constructor_<>(meta::constructor_policy::as_raw_pointer)
            .constructor_<Entity*>(meta::constructor_policy::as_raw_pointer)
            .member_("Mesh", &MeshRenderer::Mesh)
        ;

        meta::class_<ScriptComponent>(meta::metadata_()
            ("Name"s, "ScriptComponent"s))
            .constructor_<>(meta::constructor_policy::as_raw_pointer)
            .constructor_<Entity*>(meta::constructor_policy::as_raw_pointer)
        ;

        meta::static_scope_("Components")
            .typedef_<MeshRenderer>("MeshRenderer")
            .typedef_<ScriptComponent>("ScriptComponent")
            .typedef_<Camera>("Camera")
            .typedef_<PointLight>("PointLight")
        ;

        // endregion Components
    }

    struct S
    {
        S() { Register(); }
    };
}

static const S auto_register;