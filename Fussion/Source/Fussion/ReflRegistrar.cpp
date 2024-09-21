#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Components/Camera.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"
#include "Fussion/Scene/Entity.h"
#include "Math/Color.h"

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
                    ("Name"s, "Vector2"s)
                    ("ScriptFlags"s, asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS))
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
                    ("Name"s, "Vector3"s)
                    ("ScriptFlags"s, asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS))
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

        REGISTER_ENUM(AssetType);

        meta::class_<Asset>(metadata_()
                ("Name"s, "Asset"s))
            .method_("GetType"s, &Asset::type)
            .method_("GetHandle"s, &Asset::handle)
            .member_("m_Handle"s, &Asset::m_handle);

        meta::class_<Entity>(metadata_()
                ("Name"s, "Entity"s)
                ("ScriptFlags"s, asOBJ_APP_CLASS_CAK))
            .member_("m_Parent", &Entity::m_parent)
            .member_("m_Handle", &Entity::m_handle)
            .member_("m_Enabled", &Entity::m_enabled);

        // region Components
        {
            meta::class_<Component>();

            meta::class_<PointLight>(metadata_()
                    ("Name"s, "PointLight"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Offset", &PointLight::offset, as_pointer)
                .member_("Radius", &PointLight::radius, as_pointer);

            meta::class_<DirectionalLight>(metadata_()
                    ("Name"s, "DirectionalLight"s))
                .constructor_<Entity*>(as_raw_pointer)
                .member_("LightColor", &DirectionalLight::light_color, as_pointer)
                .member_("SplitLambda", &DirectionalLight::split_lambda, as_pointer);

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
                .member_("Mesh"s, &MeshRenderer::model, as_pointer)
                .member_("Materials"s, &MeshRenderer::materials, as_pointer, metadata_()("vector"s, true));

            meta::class_<ScriptComponent>(metadata_()
                    ("Name"s, "ScriptComponent"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("ClassName"s, &ScriptComponent::class_name, as_pointer);

            meta::class_<MoverComponent>(metadata_()
                    ("Name"s, "MoverComponent"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("Speed"s, &MoverComponent::speed, as_pointer);

            meta::class_<DebugDrawer>(metadata_()
                    ("Name"s, "DebugDrawer"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer)
                .member_("DrawType"s, &DebugDrawer::draw_type, as_pointer)
                .member_("Size"s, &DebugDrawer::size, as_pointer);

            meta::class_<Environment>(metadata_()
                    ("Name"s, "Environment"s))
                .constructor_<>(as_raw_pointer)
                .constructor_<Entity*>(as_raw_pointer);

            meta::static_scope_("Components")
                .typedef_<MeshRenderer>("MeshRenderer"s)
                .typedef_<ScriptComponent>("ScriptComponent"s)
                .typedef_<Camera>("Camera"s)
                .typedef_<MoverComponent>("MoverComponent"s)
                .typedef_<PointLight>("PointLight"s)
                .typedef_<DebugDrawer>("DebugDrawer"s)
                .typedef_<DirectionalLight>("DirectionalLight"s)
                .typedef_<Environment>("Environment"s);
        }
        // endregion Components

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

        meta::class_<Debug>(metadata_()("Name"s, "Debug"s))
            .function_("DrawLine"s, Debug::draw_line)
            .function_("DrawCube"s, static_cast<void(*)(Vector3, Vector3, f32, Color)>(Debug::draw_cube));

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
            .function_("IsKeyPressed"s, Input::is_key_pressed)
            .function_("IsKeyReleased"s, Input::is_key_released)
            .function_("IsKeyDown"s, Input::is_key_down)
            .function_("IsKeyUp"s, Input::is_key_up)
            .function_("IsMouseButtonPressed"s, Input::is_mouse_button_pressed)
            .function_("IsMouseButtonReleased"s, Input::is_mouse_button_released)
            .function_("IsMouseButtonDown"s, Input::is_mouse_button_down)
            .function_("IsMouseButtonUp"s, Input::is_mouse_button_up)
            .function_("GetAxis"s, Input::get_axis);

        REGISTER_ENUM(DebugDrawer::Type);
        REGISTER_ENUM(MouseButton)
        REGISTER_ENUM(Keys)

        REGISTER_ENUM(GPU::TextureFormat);

        meta::class_<AssetMetadata>(metadata_()("Name"s, "AssetMetadata"s));

        meta::class_<Texture2DMetadata>(metadata_()("Name"s, "Texture2DMetadata"s))
            .constructor_<>(as_raw_pointer)
            .member_("IsNormalMap", &Texture2DMetadata::is_normal_map, as_pointer)
            // .member_("Filter", &Texture2DMetadata::Filter, as_pointer)
            // .member_("Wrap", &Texture2DMetadata::Wrap, as_pointer)
            .member_("Format", &Texture2DMetadata::format, as_pointer);
    }
}

static const Fussion::ReflRegistrar g_Registrar;
