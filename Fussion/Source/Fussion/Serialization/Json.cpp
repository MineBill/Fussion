#include "FussionPCH.h"
#include "Json.h"

#include "Fussion/Assets/Asset.h"
#include "Fussion/Assets/AssetRef.h"
#include "Scene/Components/MeshRenderer.h"

namespace Fussion {
    auto SerializeNativeClass(meta_hpp::class_type component_type, meta_hpp::uvalue ptr) -> ordered_json
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
            } else if (value.is<Vector2*>()) {
                m = ToJson(*value.as<Vector2*>());
            } else if (value.is<Vector3*>()) {
                m = ToJson(*value.as<Vector3*>());
            } else if (value.is<Vector4*>()) {
                m = ToJson(*value.as<Vector4*>());
            } else if (value.is<Color*>()) {
                m = ToJson(*value.as<Color*>());
            } else if (data_type.is_class()) {
                auto class_type = data_type.as_class();
                if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
                    auto m_Handle = class_type.get_member("m_Handle");
                    LOG_DEBUGF("Serializing asset handle {}", m_Handle.get(value).as<AssetHandle>());
                    m = CAST(u64, m_Handle.get(value).as<AssetHandle>());
                }
            } else if (data_type.is_enum()) {
                auto enum_type = data_type.as_enum();
                auto enum_value = enum_type.value_to_evalue(*value);
                VERIFY(enum_value.is_valid());
                m = enum_value.get_name();
            }
        }
        return j;
    }

    void DeserializeClassFromJson(json j, meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
    {
        auto name = component_type.get_metadata().at("Name").as<std::string>();
        for (auto const& xx : j.items()) {
            auto& key = xx.key();
            auto& value = xx.value();

            if (auto member = component_type.get_member(key); member.is_valid()) {
                auto mem_value = member.get(ptr);
                auto data_type = mem_value.get_type().as_pointer().get_data_type();

                // @note Is this retarded?
                if (mem_value.is<s8*>()) {
                    member.set(ptr, value.get<s8>());
                } else if (mem_value.is<s16*>()) {
                    member.set(ptr, value.get<s16>());
                } else if (mem_value.is<s32*>()) {
                    member.set(ptr, value.get<s32>());
                } else if (mem_value.is<s64*>()) {
                    member.set(ptr, value.get<s64>());
                } else if (mem_value.is<u8*>()) {
                    member.set(ptr, value.get<u8>());
                } else if (mem_value.is<u16*>()) {
                    member.set(ptr, value.get<u16>());
                } else if (mem_value.is<u32*>()) {
                    member.set(ptr, value.get<u32>());
                } else if (mem_value.is<f32*>()) {
                    member.set(ptr, value.get<f32>());
                } else if (mem_value.is<f64*>()) {
                    member.set(ptr, value.get<f64>());
                } else if (mem_value.is<bool*>()) {
                    member.set(ptr, value.get<bool>());
                } else if (mem_value.is<std::string*>()) {
                    member.set(ptr, value.get<std::string>());
                } else if (mem_value.is<Vector2*>()) {
                    member.set(ptr, value.get<Vector2>());
                } else if (mem_value.is<Vector3*>()) {
                    member.set(ptr, value.get<Vector3>());
                } else if (mem_value.is<Vector4*>()) {
                    member.set(ptr, value.get<Vector4>());
                } else if (mem_value.is<Color*>()) {
                    member.set(ptr, value.get<Color>());
                } else if (data_type.is_class()) {
                    auto class_type = data_type.as_class();
                    if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
                        auto handle = class_type.get_member("m_Handle");
                        handle.set(mem_value, AssetHandle(value.get<u64>()));
                    }
                } else if (data_type.is_enum()) {
                    auto enum_type = data_type.as_enum();
                    auto enum_value = enum_type.name_to_evalue(value.get<std::string>());
                    member.set(ptr, enum_value.get_value());
                }
            }
        }
    }
}
