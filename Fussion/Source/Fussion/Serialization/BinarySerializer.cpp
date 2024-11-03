#include "FussionPCH.h"
#include "BinarySerializer.h"

namespace Fussion {
    BinarySerializer::BinarySerializer(std::ostream* out)
        : m_Out(out)
    {
    }

    void BinarySerializer::Initialize()
    {
    }

    template<typename T>
    void BinarySerializer::WriteGeneric(std::string_view name, T value)
    {
        (void)name;
        m_Out->write(TRANSMUTE(char const*, &value), sizeof(T));
    }

    void BinarySerializer::Write(std::string_view name, s8 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, s16 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, s32 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, s64 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, u8 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, u16 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, u32 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, u64 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, f32 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, f64 value) { WriteGeneric(name, value); }
    void BinarySerializer::Write(std::string_view name, bool value) { WriteGeneric(name, value); }

    void BinarySerializer::Write(std::string_view name, std::string_view value)
    {
        (void)name;
        size_t size = value.size();
        m_Out->write(TRANSMUTE(char const*, &size), sizeof(size_t));
        m_Out->write(value.data(), cast<std::streamsize>(value.size()));
    }

    void BinarySerializer::Write(std::string_view name, char const* value)
    {
        (void)name;
        m_Out->write(value, cast<std::streamsize>(std::strlen(value)));
    }

    void BinarySerializer::Write(std::string_view name, ISerializable const& object)
    {
        BeginObject(name, 0);
        object.Serialize(*this);
        EndObject();
    }

    void BinarySerializer::BeginObject(std::string_view name, size_t size, SerdeOptions const& options)
    {
        (void)name;
        (void)size;
        (void)options;
    }

    void BinarySerializer::EndObject() { }

    void BinarySerializer::BeginArray(std::string_view name, size_t size)
    {
        (void)name;
        m_Out->write(TRANSMUTE(char const*, &size), sizeof(size_t));
    }

    void BinarySerializer::EndArray() { }

    void BinarySerializer::WriteByteArray(std::string_view name, u8 const* ptr, usz size)
    {
        (void)name;
        m_Out->write(TRANSMUTE(char const*, &size), sizeof(usz));
        m_Out->write(TRANSMUTE(char const*, ptr), cast<std::streamsize>(size));
    }

    /// =================================================
    /// =================================================
    /// =================================================

    BinaryDeserializer::BinaryDeserializer(std::istream* in)
        : m_In(in)
    {
    }

    template<typename T>
    bool BinaryDeserializer::GenericRead(std::string_view name, T& value)
    {
        (void)name;
        m_In->read(TRANSMUTE(char*, &value), sizeof(T));
        return true;
    }

    void BinaryDeserializer::Initialize()
    {
    }

    bool BinaryDeserializer::Read(std::string_view name, s8& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, s16& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, s32& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, s64& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, u8& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, u16& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, u32& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, u64& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, f32& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, f64& value) { return GenericRead(name, value); }
    bool BinaryDeserializer::Read(std::string_view name, bool& value) { return GenericRead(name, value); }

    bool BinaryDeserializer::Read(std::string_view name, std::string& value)
    {
        (void)name;
        size_t size;
        m_In->read(TRANSMUTE(char*, &size), sizeof(size_t));
        value.resize(size);
        m_In->read(value.data(), cast<std::streamsize>(size));
        return true;
    }

    bool BinaryDeserializer::Read(std::string_view name, ISerializable& object)
    {
        if (size_t size; BeginObject(name, size)) {
            object.Deserialize(*this);
            EndObject();
        }
        return true;
    }

    bool BinaryDeserializer::ReadByteArray(std::string_view name, u8* ptr, size_t size)
    {
        (void)name;
        m_In->read(TRANSMUTE(char*, ptr), cast<std::streamsize>(size));
        return true;
    }

    bool BinaryDeserializer::BeginObject(std::string_view name, size_t& size)
    {
        (void)name;
        (void)size;
        return true;
    }

    void BinaryDeserializer::EndObject() { }

    void BinaryDeserializer::BeginArray(std::string_view name, size_t& size)
    {
        (void)name;
        m_In->read(TRANSMUTE(char*, &size), sizeof(size_t));
    }

    void BinaryDeserializer::EndArray() { }

    auto BinaryDeserializer::ReadKeys() -> std::vector<std::string>
    {
        return {};
    }
}
