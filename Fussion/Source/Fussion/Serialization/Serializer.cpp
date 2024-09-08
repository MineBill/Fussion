#include "FussionPCH.h"
#include "Serializer.h"

#include "Core/Uuid.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Color.h"

namespace Fussion {
    void Serializer::write(std::string_view name, Vector2 const& value)
    {
        begin_object(name, 2);
        write("x", value.x);
        write("y", value.y);
        end_object();
    }

    void Serializer::write(std::string_view name, Vector3 const& value)
    {
        begin_object(name, 3);
        write("x", value.x);
        write("y", value.y);
        write("z", value.z);
        end_object();
    }

    void Serializer::write(std::string_view name, Vector4 const& value)
    {
        begin_object(name, 4);
        write("x", value.x);
        write("y", value.y);
        write("z", value.z);
        write("w", value.w);
        end_object();
    }

    void Serializer::write(std::string_view name, Color const& value)
    {
        begin_object(name, 4);
        write("r", value.r);
        write("g", value.g);
        write("b", value.b);
        write("a", value.a);
        end_object();
    }

    void Serializer::write(std::string_view name, Uuid const& value)
    {
        write(name, CAST(u64, value));
    }

    void Serializer::write(std::string_view name, std::string const& value)
    {
        write(name, std::string_view(value));
    }

    void Serializer::write(std::string_view name, std::filesystem::path const& path)
    {
        write(name, path.string());
    }

    void Deserializer::read(std::string_view name, Vector2& value)
    {
        size_t size;
        if (begin_object(name, size)) {
            read("x", value.x);
            read("y", value.y);
            end_object();
        }
    }

    void Deserializer::read(std::string_view name, Vector3& value)
    {
        size_t size;
        if (begin_object(name, size)) {
            read("x", value.x);
            read("y", value.y);
            read("z", value.z);
            end_object();
        }
    }

    void Deserializer::read(std::string_view name, Vector4& value)
    {
        size_t size;
        if (begin_object(name, size)) {
            read("x", value.x);
            read("y", value.y);
            read("z", value.z);
            read("w", value.w);
            end_object();
        }
    }

    void Deserializer::read(std::string_view name, Color& value)
    {
        size_t size;
        if (begin_object(name, size)) {
            read("r", value.r);
            read("g", value.g);
            read("b", value.b);
            read("a", value.a);
            end_object();
        }
    }

    void Deserializer::read(std::string_view name, Uuid& value)
    {
        u64 id;
        read(name, id);
        value = Uuid(id);
    }

    void Deserializer::read(std::string_view name, std::filesystem::path& p)
    {
        std::string s;
        read(name, s);
        p = s;
    }
}
