#include "FussionPCH.h"
#include "BinarySerializer.h"

namespace Fussion {
    BinarySerializer::BinarySerializer(std::ostream* out)
        : m_out(out)
    {
    }

    void BinarySerializer::initialize()
    {
    }

    template<typename T>
    void BinarySerializer::write_generic(std::string_view name, T value)
    {
        (void)name;
        m_out->write(TRANSMUTE(char const*, &value), sizeof(T));
    }

    void BinarySerializer::write(std::string_view name, s8 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, s16 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, s32 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, s64 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, u8 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, u16 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, u32 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, u64 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, f32 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, f64 value) { write_generic(name, value); }
    void BinarySerializer::write(std::string_view name, bool value) { write_generic(name, value); }

    void BinarySerializer::write(std::string_view name, std::string_view value)
    {
        (void)name;
        size_t size = value.size();
        m_out->write(TRANSMUTE(char const*, &size), sizeof(size_t));
        m_out->write(value.data(), cast<std::streamsize>(value.size()));
    }

    void BinarySerializer::write(std::string_view name, char const* value)
    {
        (void)name;
        m_out->write(value, cast<std::streamsize>(std::strlen(value)));
    }

    void BinarySerializer::write(std::string_view name, ISerializable const& object)
    {
        begin_object(name, 0);
        object.serialize(*this);
        end_object();
    }

    void BinarySerializer::begin_object(std::string_view name, size_t size, SerdeOptions const& options)
    {
        (void)name;
        (void)size;
        (void)options;
    }

    void BinarySerializer::end_object() { }

    void BinarySerializer::begin_array(std::string_view name, size_t size)
    {
        (void)name;
        m_out->write(TRANSMUTE(char const*, &size), sizeof(size_t));
    }

    void BinarySerializer::end_array() { }

    void BinarySerializer::write_byte_array(std::string_view name, u8 const* ptr, usz size)
    {
        (void)name;
        m_out->write(TRANSMUTE(char const*, &size), sizeof(usz));
        m_out->write(TRANSMUTE(char const*, ptr), cast<std::streamsize>(size));
    }

    /// =================================================
    /// =================================================
    /// =================================================

    BinaryDeserializer::BinaryDeserializer(std::istream* in)
        : m_in(in)
    {
    }

    template<typename T>
    bool BinaryDeserializer::generic_read(std::string_view name, T& value)
    {
        (void)name;
        m_in->read(TRANSMUTE(char*, &value), sizeof(T));
        return true;
    }

    void BinaryDeserializer::initialize()
    {
    }

    bool BinaryDeserializer::read(std::string_view name, s8& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, s16& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, s32& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, s64& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, u8& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, u16& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, u32& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, u64& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, f32& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, f64& value) { return generic_read(name, value); }
    bool BinaryDeserializer::read(std::string_view name, bool& value) { return generic_read(name, value); }

    bool BinaryDeserializer::read(std::string_view name, std::string& value)
    {
        (void)name;
        size_t size;
        m_in->read(TRANSMUTE(char*, &size), sizeof(size_t));
        value.resize(size);
        m_in->read(value.data(), cast<std::streamsize>(size));
        return true;
    }

    bool BinaryDeserializer::read(std::string_view name, ISerializable& object)
    {
        if (size_t size; begin_object(name, size)) {
            object.deserialize(*this);
            end_object();
        }
        return true;
    }

    bool BinaryDeserializer::read_byte_array(std::string_view name, u8* ptr, size_t size)
    {
        (void)name;
        m_in->read(TRANSMUTE(char*, ptr), cast<std::streamsize>(size));
        return true;
    }

    bool BinaryDeserializer::begin_object(std::string_view name, size_t& size)
    {
        (void)name;
        (void)size;
        return true;
    }

    void BinaryDeserializer::end_object() { }

    void BinaryDeserializer::begin_array(std::string_view name, size_t& size)
    {
        (void)name;
        m_in->read(TRANSMUTE(char*, &size), sizeof(size_t));
    }

    void BinaryDeserializer::end_array() { }

    auto BinaryDeserializer::keys() -> std::vector<std::string>
    {
        return {};
    }
}
