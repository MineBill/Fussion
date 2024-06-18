#pragma once
#include "Fussion/Serialization/json.hpp"

namespace Fussion {
    using json = nlohmann::json;
    using ordered_json = nlohmann::ordered_json;

    inline json ToJson(Vector3 const& vec)
    {
        return json{
            { "x", vec.x },
            { "y", vec.y },
            { "z", vec.z },
        };
    }

    inline Vector3 Vec3FromJson(const json& j)
    {
        Vector3 vec;
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
        return vec;
    }

    inline json ToJson(Vector2 const& vec)
    {
        return json{
            { "x", vec.x },
            { "y", vec.y },
        };
    }

    inline Vector2 Vec2FromJson(const json& j)
    {
        Vector2 vec;
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        return vec;
    }


    inline void to_json(json& j, UUID const& id)
    {
        j = CAST(u64, id);
    }

    inline void from_json(const json& j, UUID& id)
    {
        id = UUID(j.get<u64>());
    }
}

inline void to_json(Fussion::json& j, Vector3 const& v)
{
    j = Fussion::json{
        { "x", v.x },
        { "y", v.y },
        { "z", v.z },
    };
}

inline void from_json(const Fussion::json& j, Vector3& v)
{
    v = Vector3(j.value("x", 0.0), j.value("y", 0.0), j.value("z", 0.0));
}
