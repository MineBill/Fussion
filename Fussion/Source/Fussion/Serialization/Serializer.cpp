#include "FussionPCH.h"
#include "Serializer.h"

#include "Core/Uuid.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Color.h"

namespace Fussion {
    void Serializer::Write(std::string_view name, Vector2 const& value)
    {
        BeginObject(name, 2);
        Write("X", value.X);
        Write("Y", value.Y);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Vector3 const& value)
    {
        BeginObject(name, 3);
        Write("X", value.X);
        Write("Y", value.Y);
        Write("Z", value.Z);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Vector4 const& value)
    {
        BeginObject(name, 4);
        Write("X", value.X);
        Write("Y", value.Y);
        Write("Z", value.Z);
        Write("W", value.W);
        EndObject();
    }

    void Serializer::Write(std::string_view name, Color const& value)
    {
        BeginObject(name, 4);
        Write("R", value.R);
        Write("G", value.G);
        Write("B", value.B);
        Write("A", value.A);
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
            Read("X", value.X);
            Read("Y", value.Y);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Vector3& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("X", value.X);
            Read("Y", value.Y);
            Read("Z", value.Z);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Vector4& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("X", value.X);
            Read("Y", value.Y);
            Read("Z", value.Z);
            Read("W", value.W);
            EndObject();
        }
    }

    void Deserializer::Read(std::string_view name, Color& value)
    {
        size_t size;
        if (BeginObject(name, size)) {
            Read("R", value.R);
            Read("G", value.G);
            Read("B", value.B);
            Read("A", value.A);
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
