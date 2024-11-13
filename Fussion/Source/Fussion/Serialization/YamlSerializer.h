#pragma once
#include <Fussion/Serialization/Serializer.h>

#include <yaml-cpp/yaml.h>

#include <stack>

namespace Fussion {
    class YamlSerializer final : public Serializer {
    public:
        virtual void initialize() override;
        virtual void write(std::string_view name, s8 value) override;
        virtual void write(std::string_view name, s16 value) override;
        virtual void write(std::string_view name, s32 value) override;
        virtual void write(std::string_view name, s64 value) override;
        virtual void write(std::string_view name, u8 value) override;
        virtual void write(std::string_view name, u16 value) override;
        virtual void write(std::string_view name, u32 value) override;
        virtual void write(std::string_view name, u64 value) override;
        virtual void write(std::string_view name, f32 value) override;
        virtual void write(std::string_view name, f64 value) override;
        virtual void write(std::string_view name, bool value) override;
        virtual void write(std::string_view name, std::string_view value) override;
        virtual void write(std::string_view name, char const* value) override;
        virtual void write(std::string_view name, ISerializable const& object) override;
        virtual void begin_object(std::string_view name, size_t size, SerdeOptions const& options = {}) override;
        virtual void end_object() override;
        virtual void begin_array(std::string_view name, size_t size) override;
        virtual void end_array() override;

        virtual void write_byte_array(std::string_view name, u8 const* ptr, usz size) override;

        std::string to_string();

    private:
        template<typename T>
        void generic_write(std::string_view name, T value);

    private:
        enum class Type {
            Array,
            Object,
        };

        std::stack<Type> m_type_stack { { Type::Object } };
        YAML::Emitter m_emitter {};
    };

    class YamlDeserializer final : public Deserializer {
    public:
        using Deserializer::read;

        explicit YamlDeserializer(std::string const& data);

        virtual void initialize() override;
        virtual bool read(std::string_view name, s8& value) override;
        virtual bool read(std::string_view name, s16& value) override;
        virtual bool read(std::string_view name, s32& value) override;
        virtual bool read(std::string_view name, s64& value) override;
        virtual bool read(std::string_view name, u8& value) override;
        virtual bool read(std::string_view name, u16& value) override;
        virtual bool read(std::string_view name, u32& value) override;
        virtual bool read(std::string_view name, u64& value) override;
        virtual bool read(std::string_view name, f32& value) override;
        virtual bool read(std::string_view name, f64& value) override;
        virtual bool read(std::string_view name, bool& value) override;
        virtual bool read(std::string_view name, std::string& value) override;
        virtual bool read(std::string_view name, ISerializable& object) override;
        virtual bool read_byte_array(std::string_view name, u8* ptr, size_t size) override;

        virtual bool begin_object(std::string_view name, size_t& size) override;
        virtual void end_object() override;
        virtual void begin_array(std::string_view name, size_t& size) override;
        virtual void end_array() override;
        virtual auto keys() -> std::vector<std::string> override;

    private:
        template<typename T>
        bool generic_read(std::string_view name, T& value);

        std::stack<YAML::Node> m_nodes {};
        std::stack<u32> m_index_stack {};
    };

}
