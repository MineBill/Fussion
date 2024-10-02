#pragma once
#include <Fussion/Serialization/ISerializable.h>
#include <Fussion/Core/Concepts.h>
#include <Fussion/Core/Types.h>

#include <string_view>
#include <vector>
#include <magic_enum/magic_enum.hpp>
#include <filesystem>

namespace Fussion {
    class Uuid;
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Color;

    /// Serialization provider interface.
    /// This interface can be used to provide a serialization backend for ISerializable types.
    class Serializer {
    public:
        virtual ~Serializer() = default;

        virtual void initialize() = 0;

        virtual void write(std::string_view name, s8 value) = 0;
        virtual void write(std::string_view name, s16 value) = 0;
        virtual void write(std::string_view name, s32 value) = 0;
        virtual void write(std::string_view name, s64 value) = 0;

        virtual void write(std::string_view name, u8 value) = 0;
        virtual void write(std::string_view name, u16 value) = 0;
        virtual void write(std::string_view name, u32 value) = 0;
        virtual void write(std::string_view name, u64 value) = 0;

        virtual void write(std::string_view name, f32 value) = 0;
        virtual void write(std::string_view name, f64 value) = 0;

        virtual void write(std::string_view name, bool value) = 0;
        virtual void write(std::string_view name, std::string_view value) = 0;
        virtual void write(std::string_view name, char const* value) = 0;


        virtual void write(std::string_view name, ISerializable const& object) = 0;

        virtual void begin_object(std::string_view name, size_t size) = 0;
        virtual void end_object() = 0;

        virtual void begin_array(std::string_view name, size_t size) = 0;
        virtual void end_array() = 0;

        void write(std::string_view name, Vector2 const& value);
        void write(std::string_view name, Vector3 const& value);
        void write(std::string_view name, Vector4 const& value);
        void write(std::string_view name, Color const& value);
        void write(std::string_view name, Uuid const& value);
        void write(std::string_view name, std::string const& value);
        void write(std::string_view name, std::filesystem::path const& path);

        template<typename E>
            requires std::is_enum_v<E>
        void write(std::string_view name, E value)
        {
            write(name, magic_enum::enum_name(value));
        }

        template<typename T>
        void write_collection(std::string_view name, std::vector<T> const& vector)
        {
            begin_array(name, vector.size());
            if constexpr (IsInstanceOf<T, std::vector>) {
                for (auto const& element : vector) {
                    write_collection("", element);
                }
            } else {
                for (auto const& element : vector) {
                    write("", element);
                }
            }
            end_array();
        }

        // template<typename K, typename V>
        // void WriteCollection(std::string_view name, std::unordered_map<K, V> const& map)
        // {
        //     BeginObject(name, map.size());
        //     for (auto const& [key, value] : map) {
        //         Write("", key);
        //         Write("", value);
        //     }
        //     EndObject();
        // }
    };

    /// Deserialization provider interface.
    /// This interface can be used to provide a deserialization backend for ISerializable types.
    class Deserializer {
    public:
        virtual ~Deserializer() = default;

        virtual void initialize() = 0;

        virtual void read(std::string_view name, s8& value) = 0;
        virtual void read(std::string_view name, s16& value) = 0;
        virtual void read(std::string_view name, s32& value) = 0;
        virtual void read(std::string_view name, s64& value) = 0;

        virtual void read(std::string_view name, u8& value) = 0;
        virtual void read(std::string_view name, u16& value) = 0;
        virtual void read(std::string_view name, u32& value) = 0;
        virtual void read(std::string_view name, u64& value) = 0;

        virtual void read(std::string_view name, f32& value) = 0;
        virtual void read(std::string_view name, f64& value) = 0;

        virtual void read(std::string_view name, bool& value) = 0;
        virtual void read(std::string_view name, std::string& value) = 0;

        virtual void read(std::string_view name, ISerializable& object) = 0;

        virtual bool begin_object(std::string_view name, size_t& size) = 0;
        virtual void end_object() = 0;

        virtual void begin_array(std::string_view name, size_t& size) = 0;
        virtual void end_array() = 0;

        void read(std::string_view name, Vector2& value);
        void read(std::string_view name, Vector3& value);
        void read(std::string_view name, Vector4& value);
        void read(std::string_view name, Color& value);
        void read(std::string_view name, Uuid& value);
        void read(std::string_view name, std::filesystem::path& p);

        template<typename E>
            requires std::is_enum_v<E>
        void read(std::string_view name, E& value)
        {
            std::string v;
            read(name, v);
            if (auto val = magic_enum::enum_cast<E>(v)) {
                value = *val;
            }
        }

        template<typename T>
        void read_collection(std::string_view name, std::vector<T>& vector)
        {
            size_t size;
            begin_array(name, size);
            vector.resize(size);
            if constexpr (IsInstanceOf<T, std::vector>) {
                for (auto& element : vector) {
                    read_collection("", element);
                }
            } else {
                for (auto& element : vector) {
                    read("", element);
                }
            }
            end_array();
        }

        virtual auto read_keys() -> std::vector<std::string> = 0;
    };
}
