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
        { "X", vec.X },
        { "Y", vec.Y },
        { "Z", vec.Z },
    };
}

inline Vector3 Vec3FromJson(const json& j)
{
    if (!j.contains("X") || !j.contains("Y") || !j.contains("Z")) {
        return {};
    }
    Vector3 vec;
    j.at("X").get_to(vec.X);
    j.at("Y").get_to(vec.Y);
    j.at("Z").get_to(vec.Z);
    return vec;
}

inline json ToJson(Vector4 const& vec)
{
    return json{
        { "X", vec.X },
        { "Y", vec.Y },
        { "Z", vec.Z },
        { "W", vec.W },
    };
}

inline Vector4 Vec4FromJson(const json& j)
{
    if (!j.contains("X") || !j.contains("Y") || !j.contains("Z") || !j.contains("W")) {
        return {};
    }
    Vector4 vec;
    j.at("X").get_to(vec.X);
    j.at("Y").get_to(vec.Y);
    j.at("Z").get_to(vec.Z);
    j.at("W").get_to(vec.W);
    return vec;
}

inline json ToJson(Vector2 const& vec)
{
    return json{
        { "X", vec.X },
        { "Y", vec.Y },
    };
}

inline Vector2 Vec2FromJson(const json& j)
{
    Vector2 vec;
    j.at("X").get_to(vec.X);
    j.at("Y").get_to(vec.Y);
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

inline void to_json(json& j, Uuid const& id)
{
    j = CAST(u64, id);
}

inline void from_json(const json& j, Uuid& id)
{
    id = Uuid(j.get<u64>());
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
    v = Fussion::Color(j.value("R", 0.0f), j.value("G", 0.0f), j.value("B", 0.0f), j.value("A", 0.0f));
}

inline void from_json(const Fussion::json& j, Vector2& v)
{
    if (!j.contains("X") || !j.contains("Y")) {
        v = Vector2{};
        return;
    }
    v = Vector2(j.value("X", 0.0), j.value("Y", 0.0));
}

inline void from_json(const Fussion::json& j, Vector3& v)
{
    if (!j.contains("X") || !j.contains("Y") || !j.contains("Z")) {
        v = Vector3{};
        return;
    }
    v = Vector3(j.value("X", 0.0), j.value("Y", 0.0), j.value("Z", 0.0));
}

inline void from_json(const Fussion::json& j, Vector4& v)
{
    if (!j.contains("X") || !j.contains("Y") || !j.contains("Z") || !j.contains("W")) {
        v = Vector4{};
        return;
    }
    v = Vector4(j.value("X", 0.0), j.value("Y", 0.0), j.value("Z", 0.0), j.value("W", 0.0));
}

inline void to_json(Fussion::json& j, Vector2 const& v)
{
    j = Fussion::json{
        { "X", v.X },
        { "Y", v.Y },
    };
}

inline void to_json(Fussion::json& j, Vector3 const& v)
{

    j = Fussion::json{
        { "X", v.X },
        { "Y", v.Y },
        { "Z", v.Z },
    };
}

inline void to_json(Fussion::json& j, Vector4 const& v)
{
    j = Fussion::json{
        { "X", v.X },
        { "Y", v.Y },
        { "Z", v.Z },
        { "W", v.Z },
    };
}

}
