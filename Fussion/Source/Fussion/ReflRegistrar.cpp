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

#include "Debug/Debug.h"
#include "Input/Input.h"
#include "Input/Keys.h"
#include "Scene/Scene.h"
#include "Scene/Components/DirectionalLight.h"
#include <angelscript.h>
#include <magic_enum/magic_enum.hpp>

#define REGISTER_ENUM(EnumName)                                              \
{                                                                            \
    auto ee = meta::enum_<EnumName>(meta::metadata_()("Name"s, #EnumName##s));  \
    for (auto const& [value, name] : magic_enum::enum_entries<EnumName>()) { \
        ee.evalue_(std::string{ name }, value);                              \
    }                                                                        \
}

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
        using meta::metadata_;

        using meta::constructor_policy::as_raw_pointer;
        using meta::member_policy::as_pointer;

        std::print("Registering meta info\n");

        // Math
        {
            meta::class_<Color>(metadata_()
                    ("Name"s, "Color"s))
                .member_("R", &Color::R, as_pointer)
                .member_("G", &Color::G, as_pointer)
                .member_("B", &Color::B, as_pointer)
                .member_("A", &Color::A, as_pointer)
                .method_("Darken", &Color::Darken)
                .method_("Lighten", &Color::Lighten);

            meta::class_<Vector2>(
                    metadata_()
                    ("Name"s, "Vector2"s)
                    ("ScriptFlags"s, asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS))
                .constructor_<f32, f32>()
                .constructor_<f64, f64>()
                .member_("X", &Vector2::X, as_pointer)
                .member_("Y", &Vector2::Y, as_pointer)
                .method_("Length", &Vector2::Length)
                .method_("DistanceTo", &Vector2::DistanceTo)
                .method_("LengthSquared", &Vector2::LengthSquared)
                .method_("DistanceToSquared", &Vector2::DistanceToSquared);

            meta::class_<Vector3>(
                    metadata_()
                    ("Name"s, "Vector3"s)
                    ("ScriptFlags"s, asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS))
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

        meta::class_<Asset>(metadata_()
                ("Name"s, "Asset"s))
            .method_("GetType"s, &Asset::GetType)
            .method_("GetHandle"s, &Asset::GetHandle)
            .member_("m_Handle"s, &Asset::m_Handle);

        meta::class_<Entity>(metadata_()
                ("Name"s, "Entity"s)
                ("ScriptFlags"s, asOBJ_APP_CLASS_CAK))
            .member_("m_Parent", &Entity::m_Parent)
            .member_("m_Handle", &Entity::m_Handle)
            .member_("m_Enabled", &Entity::m_Enabled);

        // region Components
        {
            meta::class_<Component>();

            meta::class_<PointLight>(metadata_()
                    ("Name"s, "PointLight"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Offset", &PointLight::Offset, as_pointer)
                .member_("Radius", &PointLight::Radius, as_pointer);

            meta::class_<DirectionalLight>(metadata_()
                    ("Name"s, "DirectionalLight"s))
                .constructor_<Entity*>(as_raw_pointer)
                .member_("LightColor", &DirectionalLight::LightColor, as_pointer);

            meta::class_<Camera>(metadata_()
                    ("Name"s, "Camera"s))
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Fov"s, &Camera::Fov, as_pointer, metadata_()
                    ("Range"s, "1, 100"s))
                .member_("Near"s, &Camera::Near, as_pointer)
                .member_("Far"s, &Camera::Far, as_pointer);

            meta::class_<MeshRenderer>(metadata_()
                    ("Name"s, "MeshRenderer"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Mesh"s, &MeshRenderer::Mesh, as_pointer)
                .member_("Material"s, &MeshRenderer::Material, as_pointer);

            meta::class_<ScriptComponent>(metadata_()
                    ("Name"s, "ScriptComponent"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("ClassName"s, &ScriptComponent::ClassName, as_pointer);

            meta::class_<MoverComponent>(metadata_()
                    ("Name"s, "MoverComponent"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Speed"s, &MoverComponent::Speed, as_pointer);

            meta::enum_<DebugDrawer::Type>()
                .evalue_("Box"s, DebugDrawer::Type::Box)
                .evalue_("Sphere"s, DebugDrawer::Type::Sphere);

            meta::class_<DebugDrawer>(metadata_()
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

        meta::class_<Scene>(metadata_()
                ("Name"s, "Scene"s))
            .method_("CreateEntity"s, &Scene::CreateEntity);

        meta::class_<AssetRefBase>(metadata_()
                ("Name"s, "AssetRefBase"s))
            .method_("Handle"s, &AssetRefBase::Handle)
            .method_("SetHandle"s, &AssetRefBase::SetHandle)
            .member_("m_Handle"s, &AssetRefBase::m_Handle)
            .method_("GetType", &AssetRefBase::GetType);

        meta::class_<PbrMaterial>(metadata_()
                ("Name"s, "PbrMaterial"s))
            .constructor_<>()
            .function_("GetStaticType", &PbrMaterial::GetStaticType)
            .member_("ObjectColor", &PbrMaterial::ObjectColor, as_pointer)
            .member_("Metallic", &PbrMaterial::Metallic, as_pointer)
            .member_("Roughness", &PbrMaterial::Roughness, as_pointer);

        meta::class_<Debug>(metadata_()("Name"s, "Debug"s))
            .function_("DrawLine"s, Debug::DrawLine)
            .function_("DrawCube"s, Debug::DrawCube);

        meta::static_scope_("Scripting")
            .typedef_<Vector2>("Vector2"s)
            .typedef_<Vector3>("Vector3"s)
            .typedef_<Entity>("Entity"s)
            .typedef_<Scene>("Scene"s)
            .typedef_<Debug>("Debug"s)
            .typedef_<MeshRenderer>("MeshRenderer"s)
            .typedef_<ScriptComponent>("ScriptComponent"s)
            .typedef_<Camera>("Camera"s)
            .typedef_<MoverComponent>("MoverComponent"s)
            .typedef_<PointLight>("PointLight"s)
            .typedef_<DebugDrawer>("DebugDrawer"s)
            .typedef_<DirectionalLight>("DirectionalLight"s);

        meta::class_<Input>(metadata_()("Name"s, "Input"s))
            .function_("IsKeyPressed"s, Input::IsKeyPressed)
            .function_("IsKeyReleased"s, Input::IsKeyReleased)
            .function_("IsKeyDown"s, Input::IsKeyDown)
            .function_("IsKeyUp"s, Input::IsKeyUp)
            .function_("IsMouseButtonPressed"s, Input::IsMouseButtonPressed)
            .function_("IsMouseButtonReleased"s, Input::IsMouseButtonReleased)
            .function_("IsMouseButtonDown"s, Input::IsMouseButtonDown)
            .function_("IsMouseButtonUp"s, Input::IsMouseButtonUp)
            .function_("GetAxis"s, Input::GetAxis);

        REGISTER_ENUM(MouseButton)
        REGISTER_ENUM(Keys)
        REGISTER_ENUM(RHI::ImageFormat)
        REGISTER_ENUM(RHI::FilterMode)
        REGISTER_ENUM(RHI::WrapMode)

        meta::class_<AssetMetadata>(metadata_()("Name"s, "AssetMetadata"s));

        meta::class_<Texture2DMetadata>(metadata_()("Name"s, "Texture2DMetadata"s))
            .constructor_<>(as_raw_pointer)
            .member_("IsNormalMap", &Texture2DMetadata::IsNormalMap, as_pointer)
            .member_("Filter", &Texture2DMetadata::Filter, as_pointer)
            .member_("Wrap", &Texture2DMetadata::Wrap, as_pointer)
            .member_("Format", &Texture2DMetadata::Format, as_pointer);
    }
}

static const Fussion::ReflRegistrar g_Registrar;
