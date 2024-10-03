#include "FussionPCH.h"
#include "Serializer.h"

#include "Core/Uuid.h"
#include "Math/Color.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

namespace Fussion {
    void Serializer::Write(std::string_view name, Vector2 const& value)
    {
        BeginObject(name, 2);
        Write("x", value.x);
        Write("y", value.y);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Vector3 const& value)
    {
        BeginObject(name, 3);
        Write("x", value.x);
        Write("y", value.y);
        Write("z", value.z);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Vector4 const& value)
    {
        BeginObject(name, 4);
        Write("x", value.x);
        Write("y", value.y);
        Write("z", value.z);
        Write("w", value.w);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Color const& value)
    {
        BeginObject(name, 4);
        Write("r", value.r);
        Write("g", value.g);
        Write("b", value.b);
        Write("a", value.a);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Uuid const& value)
    {
        Write(name, CAST(u64, value));
    }

    void Serializer::Write(std::string_view name, std::string const& value)
    {
        Write(name, std::string_view(value));
    }

    void Serializer::Write(std::string_view name, std::filesystem::path const& path)
    {
        Write(name, path.string());
    }

    void Deserializer::Read(std::string_view name, Vector2& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("x", value.x);
            Read("y", value.y);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Vector3& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("x", value.x);
            Read("y", value.y);
            Read("z", value.z);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Vector4& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("x", value.x);
            Read("y", value.y);
            Read("z", value.z);
            Read("w", value.w);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Color& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("r", value.r);
            Read("g", value.g);
            Read("b", value.b);
            Read("a", value.a);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Uuid& value)
    {
        u64 id;
        Read(name, id);
        value = Uuid(id);
    }

    void Deserializer::Read(std::string_view name, std::filesystem::path& p)
    {
        std::string s;
        Read(name, s);
        p = s;
    }
}
