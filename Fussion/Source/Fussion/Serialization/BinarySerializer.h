#pragma once
#include "Fussion/Serialization/Serializer.h"

namespace Fussion {
    class BinarySerializer final : public Serializer {
    public:
        explicit BinarySerializer(std::ostream* out);

        virtual void Initialize() override;
        virtual void Write(std::string_view name, s8 value) override;
        virtual void Write(std::string_view name, s16 value) override;
        virtual void Write(std::string_view name, s32 value) override;
        virtual void Write(std::string_view name, s64 value) override;
        virtual void Write(std::string_view name, u8 value) override;
        virtual void Write(std::string_view name, u16 value) override;
        virtual void Write(std::string_view name, u32 value) override;
        virtual void Write(std::string_view name, u64 value) override;
        virtual void Write(std::string_view name, f32 value) override;
        virtual void Write(std::string_view name, f64 value) override;
        virtual void Write(std::string_view name, bool value) override;
        virtual void Write(std::string_view name, std::string_view value) override;
        virtual void Write(std::string_view name, char const* value) override;
        virtual void Write(std::string_view name, ISerializable const& object) override;
        virtual void BeginObject(std::string_view name, size_t size, SerdeOptions const& options = {}) override;
        virtual void EndObject() override;
        virtual void BeginArray(std::string_view name, size_t size) override;
        virtual void EndArray() override;
        virtual void WriteByteArray(std::string_view name, u8 const* ptr, usz size) override;

    private:
        template<typename T>
        void WriteGeneric(std::string_view name, T value);
        std::ostream* m_Out {};
    };

    class BinaryDeserializer final : public Deserializer {
    public:
        explicit BinaryDeserializer(std::istream* in);

        virtual void Initialize() override;
        virtual bool Read(std::string_view name, s8& value) override;
        virtual bool Read(std::string_view name, s16& value) override;
        virtual bool Read(std::string_view name, s32& value) override;
        virtual bool Read(std::string_view name, s64& value) override;
        virtual bool Read(std::string_view name, u8& value) override;
        virtual bool Read(std::string_view name, u16& value) override;
        virtual bool Read(std::string_view name, u32& value) override;
        virtual bool Read(std::string_view name, u64& value) override;
        virtual bool Read(std::string_view name, f32& value) override;
        virtual bool Read(std::string_view name, f64& value) override;
        virtual bool Read(std::string_view name, bool& value) override;
        virtual bool Read(std::string_view name, std::string& value) override;
        virtual bool Read(std::string_view name, ISerializable& object) override;
        virtual bool BeginObject(std::string_view name, size_t& size) override;
        virtual void EndObject() override;
        virtual void BeginArray(std::string_view name, size_t& size) override;
        virtual void EndArray() override;
        virtual auto ReadKeys() -> std::vector<std::string> override;

    private:
        template<typename T>
        bool GenericRead(std::string_view name, T& value);
        std::istream* m_In {};
    };
}
