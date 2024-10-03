#pragma once
#include "Json.h"
#include "Serializer.h"
#include <stack>

namespace Fussion {
    class JsonSerializer final : public Serializer {
    public:
        virtual ~JsonSerializer() override = default;

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

        virtual void BeginObject(std::string_view name, size_t size) override;
        virtual void EndObject() override;
        virtual void BeginArray(std::string_view name, size_t size) override;
        virtual void EndArray() override;

        void PopObject();

        std::string ToString();

    private:
        template<typename T>
        void GenericWrite(std::string_view name, T value);

        enum class Type {
            Array,
            Object,
        };

        std::stack<Type> m_TypeStack { { Type::Object } };
        std::stack<std::string> m_Names {};
        std::stack<u32> m_IndexStack {};
        std::stack<nlohmann::ordered_json> m_ObjectStack {};
    };

    class JsonDeserializer final : public Deserializer {
    public:
        JsonDeserializer() = default;
        explicit JsonDeserializer(std::string const& data);

        static JsonDeserializer FromJsonObject(nlohmann::json const& json);

        virtual void Initialize() override;
        virtual void Read(std::string_view name, s8& value) override;
        virtual void Read(std::string_view name, s16& value) override;
        virtual void Read(std::string_view name, s32& value) override;
        virtual void Read(std::string_view name, s64& value) override;
        virtual void Read(std::string_view name, u8& value) override;
        virtual void Read(std::string_view name, u16& value) override;
        virtual void Read(std::string_view name, u32& value) override;
        virtual void Read(std::string_view name, u64& value) override;
        virtual void Read(std::string_view name, f32& value) override;
        virtual void Read(std::string_view name, f64& value) override;
        virtual void Read(std::string_view name, bool& value) override;

        virtual void Read(std::string_view name, std::string& value) override;

        virtual void Read(std::string_view name, ISerializable& object) override;

        virtual bool BeginObject(std::string_view name, size_t& size) override;
        virtual void EndObject() override;
        virtual void BeginArray(std::string_view name, size_t& size) override;
        virtual void EndArray() override;

        virtual auto ReadKeys() -> std::vector<std::string> override;

    private:
        template<typename T>
        void GenericRead(std::string_view name, T& value);

        enum class Type {
            Array,
            Object,
        };

        std::stack<Type> m_TypeStack { { Type::Object } };
        std::stack<std::string> m_Names {};
        std::stack<u32> m_IndexStack {};
        std::stack<nlohmann::ordered_json> m_ObjectStack {};
    };
}
