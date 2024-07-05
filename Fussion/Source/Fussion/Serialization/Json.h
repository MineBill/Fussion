#pragma once
#include "Fussion/Math/Color.h"
#include "Fussion/Serialization/json.hpp"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector3.h"
#include "Fussion/Math/Vector4.h"

namespace Fussion {
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

inline json ToJson(Vector3 const& vec)
{
    return json{
        { "x", vec.X },
        { "y", vec.Y },
        { "z", vec.Z },
    };
}

inline Vector3 Vec3FromJson(const json& j)
{
    Vector3 vec;
    j.at("x").get_to(vec.X);
    j.at("y").get_to(vec.Y);
    j.at("z").get_to(vec.Z);
    return vec;
}

inline json ToJson(Vector2 const& vec)
{
    return json{
        { "x", vec.X },
        { "y", vec.Y },
    };
}

inline Vector2 Vec2FromJson(const json& j)
{
    Vector2 vec;
    j.at("x").get_to(vec.X);
    j.at("y").get_to(vec.Y);
    return vec;
}

inline json ToJson(Color const& col)
{
    return json{
        { "R", col.R },
        { "G", col.G },
        { "B", col.B },
        { "A", col.A },
    };
}

inline Color ColorFromJson(const json& j)
{
    Color col;
    j.at("A").get_to(col.R);
    j.at("G").get_to(col.G);
    j.at("B").get_to(col.B);
    j.at("A").get_to(col.A);
    return col;
}

inline void to_json(json& j, UUID const& id)
{
    j = CAST(u64, id);
}

inline void from_json(const json& j, UUID& id)
{
    id = UUID(j.get<u64>());
}

inline void to_json(Fussion::json& j, Fussion::Color const& v)
{
    j = Fussion::json{
        { "R", v.R },
        { "G", v.G },
        { "B", v.B },
        { "A", v.A },
    };
}

inline void from_json(const Fussion::json& j, Fussion::Color& v)
{
    v = Fussion::Color(j.value("R", 0.0), j.value("G", 0.0), j.value("B", 0.0), j.value("A", 0.0));
}
}

inline void to_json(Fussion::json& j, Vector3 const& v)
{
    j = Fussion::json{
        { "x", v.X },
        { "y", v.Y },
        { "z", v.Z },
    };
}

inline void from_json(const Fussion::json& j, Vector3& v)
{
    v = Vector3(j.value("x", 0.0), j.value("y", 0.0), j.value("z", 0.0));
}
