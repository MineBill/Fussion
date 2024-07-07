#include "SceneSerializer.h"
#include "EditorApplication.h"

#include "Fussion/Scene/Scene.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Serialization/Json.h"

using namespace Fussion;

// void SerializeStruct(Node& root, meta_hpp::class_type type)
// {
//     // @todo Figure out a way to collect all the public fields.
//     // for (auto const& info : type_info.GetMemberInfosWithFlag("Serialize")) {
//     //     EASSERT(info.GetPropertyType() == Reflect::PropertyType::Member);
//     //
//     //     if (info.IsPrimitiveType()) {
//     //         if (info.IsType<u32>() || info.IsType<u64>()) {
//     //
//     //         } else if (info.IsType<f32>() || info.IsType<f64>()) {
//     //
//     //         } else if (info.IsType<bool>()) {
//     //
//     //         }
//     //     } else if (info.IsClassType()) {
//     //         LOG_DEBUGF("CLASS TYPE: {}",  info.GetType().GetTypeName());
//     //     }
//     // }
//     for (auto const& member: type.get_members()) {
//         Node node{member.get_name()};
//
//         auto member_type = member.get_type().get_value_type();
//         member_type.match(meta_hpp::overloaded {
//             [&](meta_hpp::member_type mtype) {
//                 LOG_DEBUGF("MTYPE");
//             },
//             [&](meta_hpp::number_type number) {
//                 LOG_DEBUGF("NUMBER_TYPE");
//             },
//             [](auto&&){},
//         });
//         root.children().emplace_back(node);
//     }
// }

ordered_json SerializeNativeComponent(meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
{
    ordered_json j;
    auto name = component_type.get_metadata().at("Name").as<std::string>();
    for (auto const& member : component_type.get_members()) {
        auto value = member.get(ptr);
        auto data_type = value.get_type().as_pointer().get_data_type();
        auto& m = j[member.get_name()];

        // @note Is this retarded?
        if (value.is<s8*>()) {
            m = *value.as<s8*>();
        } else if (value.is<s16*>()) {
            m = *value.as<s16*>();
        } else if (value.is<s32*>()) {
            m = *value.as<s32*>();
        } else if (value.is<s64*>()) {
            m = *value.as<s64*>();
        } else if (value.is<u8*>()) {
            m = *value.as<u8*>();
        } else if (value.is<u16*>()) {
            m = *value.as<u16*>();
        } else if (value.is<u32*>()) {
            m = *value.as<u32*>();
        } else if (value.is<f32*>()) {
            m = *value.as<f32*>();
        } else if (value.is<f64*>()) {
            m = *value.as<f64*>();
        } else if (value.is<bool*>()) {
            m = *value.as<bool*>();
        } else if (value.is<std::string*>()) {
            m = *value.as<std::string*>();
        } else if (data_type.is_class()) {
            auto class_type = data_type.as_class();
            if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
                auto m_Handle = class_type.get_member("m_Handle");
                m = CAST(u64, m_Handle.get(value).as<AssetHandle>());
            }
        }
    }
    return j;
}

void DeserializeNativeComponent(json j, meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
{
    auto name = component_type.get_metadata().at("Name").as<std::string>();
    for (auto const& member : component_type.get_members()) {
        auto value = member.get(ptr);
        auto data_type = value.get_type().as_pointer().get_data_type();
        auto& m = j[member.get_name()];

        // @note Is this retarded?
        if (value.is<s8*>()) {
            member.set(ptr, m.get<s8>());
        } else if (value.is<s16*>()) {
            member.set(ptr, m.get<s16>());
        } else if (value.is<s32*>()) {
            member.set(ptr, m.get<s32>());
        } else if (value.is<s64*>()) {
            member.set(ptr, m.get<s64>());
        } else if (value.is<u8*>()) {
            member.set(ptr, m.get<u8>());
        } else if (value.is<u16*>()) {
            member.set(ptr, m.get<u16>());
        } else if (value.is<u32*>()) {
            member.set(ptr, m.get<u32>());
        } else if (value.is<f32*>()) {
            member.set(ptr, m.get<f32>());
        } else if (value.is<f64*>()) {
            member.set(ptr, m.get<f64>());
        } else if (value.is<bool*>()) {
            member.set(ptr, m.get<bool>());
        } else if (value.is<std::string*>()) {
            member.set(ptr, m.get<std::string>());
        }
    }
}

void SceneSerializer::Save(AssetMetadata metadata, Ref<Asset> const& asset)
{
    auto scene = asset->As<Scene>();
    VERIFY(scene != nullptr);

    ordered_json j = {
        {"$Type", "Scene"},
        {"Name", scene->m_Name},
    };

    j["Entities"] = json::array();

    u32 i = 0;
    for (auto const& [handle, entity] : scene->m_Entities) {
        if (handle == 0) continue;

        json en = {
            {"Name", entity.Name},
            {"Handle", entity.m_Handle},
            {"Parent", entity.m_Parent},
            {"Transform", {
                {"Position", ToJson(entity.Transform.Position)},
                {"Rotation", ToJson(entity.Transform.EulerAngles)},
                {"Scale", ToJson(entity.Transform.Scale)},
            }}
        };

        for (auto const& [id, component] : entity.GetComponents()) {
            auto ptr = component->meta_poly_ptr();
            auto component_type = ptr.get_type().as_pointer().get_data_type().as_class();
            auto name = component_type.get_metadata().at("Name").as<std::string>();

            en["Components"][name] = SerializeNativeComponent(component_type, std::move(ptr));
        }

        j["Entities"][i++] = en;
    }

    auto full_path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    FileSystem::WriteEntireFile(full_path, j.dump(2));
}

Ref<Asset> SceneSerializer::Load(AssetMetadata metadata)
{
    auto const scene = MakeRef<Scene>();
    auto const path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto const text = FileSystem::ReadEntireFile(path);

    try {
        auto j = json::parse(*text, nullptr, true, true);
        scene->m_Name = j["Name"].get<std::string>();

        auto entities = j["Entities"];
        VERIFY(entities.is_array());
        for (auto const& jentity: entities) {
            if (!jentity.contains("Name") || !jentity.contains("Handle"))
                continue;

            auto name = jentity["Name"].get<std::string>();
            auto handle = jentity["Handle"].get<Fsn::UUID>();
            auto parent = jentity["Parent"].get<Fsn::UUID>();

            auto e = scene->CreateEntityWithID(handle, name, parent);

            if (jentity.contains("Transform")) {
                if (jentity["Transform"].contains("Position"))
                    e->Transform.Position = Vec3FromJson(jentity["Transform"]["Position"]);
                if (jentity["Transform"].contains("Rotation"))
                    e->Transform.EulerAngles = Vec3FromJson(jentity["Transform"]["Rotation"]);
                if (jentity["Transform"].contains("Scale"))
                    e->Transform.Scale = Vec3FromJson(jentity["Transform"]["Scale"]);
            }

            if (jentity.contains("Components")) {
                auto components_info = meta_hpp::resolve_scope("Components");
                for (auto const& [comp_name, comp_val] : jentity["Components"].items()) {
                    (void)comp_val;
                    if (auto info = components_info.get_typedef(comp_name); info.is_valid()) {
                        auto component = e->AddComponent(info.as_class());

                        auto ptr = component->meta_poly_ptr();
                        auto component_type = ptr.get_type().as_pointer().get_data_type().as_class();

                        DeserializeNativeComponent(comp_val, component_type, std::move(ptr));
                    }
                }
            }
        }

    } catch(std::exception const& e) {
        LOG_ERRORF("Exception while loading json scene: {}", e.what());
    }
    return scene;
}