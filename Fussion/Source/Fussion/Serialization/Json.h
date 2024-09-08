#pragma once
#include <Fussion/Core/Uuid.h>
#include <Fussion/Math/Color.h>
#include <Fussion/Serialization/json.hpp>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>

#include <Fussion/meta.hpp/meta_uvalue.hpp>

namespace Fussion {
    using json = nlohmann::json;
    using ordered_json = nlohmann::ordered_json;

    inline json to_json(Vector3 const& vec)
    {
        return json{
            { "x", vec.x },
            { "y", vec.y },
            { "z", vec.z },
        };
    }

    inline Vector3 vec3_from_json(json const& j)
    {
        if (!j.contains("x") || !j.contains("y") || !j.contains("z")) {
            return {};
        }
        Vector3 vec;
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
        return vec;
    }

    inline json to_json(Vector4 const& vec)
    {
        return json{
            { "x", vec.x },
            { "y", vec.y },
            { "z", vec.z },
            { "w", vec.w },
        };
    }

    inline Vector4 vec4_from_json(json const& j)
    {
        if (!j.contains("x") || !j.contains("y") || !j.contains("z") || !j.contains("w")) {
            return {};
        }
        Vector4 vec;
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
        j.at("w").get_to(vec.w);
        return vec;
    }

    inline json to_json(Vector2 const& vec)
    {
        return json{
            { "x", vec.x },
            { "y", vec.y },
        };
    }

    inline Vector2 vec2_from_json(json const& j)
    {
        Vector2 vec;
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        return vec;
    }

    inline json to_json(Color const& col)
    {
        return json{
            { "r", col.r },
            { "g", col.g },
            { "b", col.b },
            { "a", col.a },
        };
    }

    inline Color color_from_json(json const& j)
    {
        Color col;
        j.at("a").get_to(col.r);
        j.at("g").get_to(col.g);
        j.at("b").get_to(col.b);
        j.at("a").get_to(col.a);
        return col;
    }

    inline void to_json(json& j, Uuid const& id)
    {
        j = CAST(u64, id);
    }

    inline void from_json(json const& j, Uuid& id)
    {
        id = Uuid(j.get<u64>());
    }

    inline void to_json(json& j, Color const& v)
    {
        j = json{
            { "r", v.r },
            { "g", v.g },
            { "b", v.b },
            { "a", v.a },
        };
    }

    inline void from_json(json const& j, Color& v)
    {
        v = Color(j.value("r", 0.0f), j.value("g", 0.0f), j.value("b", 0.0f), j.value("a", 0.0f));
    }

    inline void from_json(json const& j, Vector2& v)
    {
        if (!j.contains("x") || !j.contains("y")) {
            v = Vector2{};
            return;
        }
        v = Vector2(j.value("x", 0.0), j.value("y", 0.0));
    }

    inline void from_json(json const& j, Vector3& v)
    {
        if (!j.contains("x") || !j.contains("y") || !j.contains("z")) {
            v = Vector3{};
            return;
        }
        v = Vector3(j.value("x", 0.0), j.value("y", 0.0), j.value("z", 0.0));
    }

    inline void from_json(json const& j, Vector4& v)
    {
        if (!j.contains("x") || !j.contains("y") || !j.contains("z") || !j.contains("w")) {
            v = Vector4{};
            return;
        }
        v = Vector4(j.value("x", 0.0), j.value("y", 0.0), j.value("z", 0.0), j.value("w", 0.0));
    }

    inline void to_json(json& j, Vector2 const& v)
    {
        j = json{
            { "x", v.x },
            { "y", v.y },
        };
    }

    inline void to_json(json& j, Vector3 const& v)
    {

        j = json{
            { "x", v.x },
            { "y", v.y },
            { "z", v.z },
        };
    }

    inline void to_json(json& j, Vector4 const& v)
    {
        j = json{
            { "x", v.x },
            { "y", v.y },
            { "z", v.z },
            { "w", v.z },
        };
    }

    auto serialize_native_class(meta_hpp::class_type component_type, meta_hpp::uvalue ptr) -> ordered_json;
    void deserialize_class_from_json(json j, meta_hpp::class_type component_type, meta_hpp::uvalue ptr);
}
