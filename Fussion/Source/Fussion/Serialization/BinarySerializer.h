#pragma once
#include "Fussion/Serialization/Serializer.h"

namespace Fussion {
    class BinarySerializer final : public Serializer {
    public:
        explicit BinarySerializer(std::ostream* out);

        virtual void Initialize() override;
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

    private:
        template<typename T>
        void write_generic(std::string_view name, T value);
        std::ostream* m_out {};
    };

    class BinaryDeserializer final : public Deserializer {
    public:
        explicit BinaryDeserializer(std::istream* in);

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

        std::istream* m_in {};
    };
}
