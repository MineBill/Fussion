#include "FussionPCH.h"
#include "JsonSerializer.h"

namespace Fussion {
    void JsonSerializer::Initialize()
    {
        m_ObjectStack.emplace();
    }

    template<typename T>
    void JsonSerializer::GenericWrite(std::string_view name, T value)
    {
        if (m_TypeStack.top() == Type::Array) {
            m_ObjectStack.top()[m_IndexStack.top()++] = value;
        } else {
            m_ObjectStack.top()[name] = value;
        }
    }

    void JsonSerializer::Write(std::string_view name, s8 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, s16 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, s32 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, s64 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, u8 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, u16 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, u32 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, u64 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, f32 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, f64 value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, bool value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, std::string_view value)
    {
        GenericWrite(name, value);
    }

    void JsonSerializer::Write(std::string_view name, char const* value)
    {
        Write(name, std::string_view(value));
    }

    void JsonSerializer::Write(std::string_view name, ISerializable const& object)
    {
        BeginObject(name, 0);
        object.Serialize(*this);
        if (false) {
            PopObject();
            return;
        }
        EndObject();
    }

    void JsonSerializer::BeginObject(std::string_view name, size_t size)
    {
        (void)size;
        m_ObjectStack.emplace();
        m_Names.emplace(name);
        m_TypeStack.emplace(Type::Object);
    }

    void JsonSerializer::EndObject()
    {
        auto top = std::move(m_ObjectStack.top());
        m_ObjectStack.pop();

        auto name = std::move(m_Names.top());
        m_Names.pop();

        m_TypeStack.pop();

        if (m_TypeStack.top() == Type::Array) {
            m_ObjectStack.top()[m_IndexStack.top()++] = top;
        } else {
            m_ObjectStack.top()[name] = top;
        }
    }

    void JsonSerializer::BeginArray(std::string_view name, size_t size)
    {
        (void)size;
        m_ObjectStack.emplace();
        m_Names.emplace(name);

        m_IndexStack.emplace(0);
        m_TypeStack.emplace(Type::Array);
    }

    void JsonSerializer::EndArray()
    {
        auto top = std::move(m_ObjectStack.top());
        m_ObjectStack.pop();

        auto name = std::move(m_Names.top());
        m_Names.pop();

        m_IndexStack.pop();
        m_TypeStack.pop();

        if (m_TypeStack.top() == Type::Array) {
            m_ObjectStack.top()[m_IndexStack.top()++] = top;
        } else {
            m_ObjectStack.top()[name] = top;
        }
    }

    void JsonSerializer::PopObject()
    {
        m_ObjectStack.pop();
        m_Names.pop();
        m_TypeStack.pop();
    }

    std::string JsonSerializer::ToString()
    {
        return m_ObjectStack.top().dump(2);
    }

    // ======================================================
    // === Deserializer =====================================
    // ======================================================

    template<typename T>
    void JsonDeserializer::GenericRead(std::string_view name, T& value)
    {
        if (m_ObjectStack.empty())
            return;
        if (m_TypeStack.top() == Type::Array) {
            value = m_ObjectStack.top().at(m_IndexStack.top()++);
        } else {
            value = m_ObjectStack.top().value(name, value);
        }
    }

    JsonDeserializer::JsonDeserializer(std::string const& data)
    {
        try {
            m_ObjectStack.emplace(nlohmann::json::parse(data, nullptr, true, true));
        } catch (std::exception const& e) {
            LOG_ERRORF("Caught exception when parsing JSON: {}", e.what());
        }
    }

    JsonDeserializer JsonDeserializer::FromJsonObject(nlohmann::json const& json)
    {
        JsonDeserializer ds {};
        ds.m_ObjectStack.emplace(nlohmann::ordered_json(json));
        return ds;
    }

    void JsonDeserializer::Initialize() { }

    void JsonDeserializer::Read(std::string_view name, s8& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, s16& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, s32& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, s64& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, u8& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, u16& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, u32& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, u64& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, f32& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, f64& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, bool& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, std::string& value)
    {
        GenericRead(name, value);
    }

    void JsonDeserializer::Read(std::string_view name, ISerializable& object)
    {
        if (size_t size; BeginObject(name, size)) {
            object.Deserialize(*this);
            EndObject();
        }
    }

    bool JsonDeserializer::BeginObject(std::string_view name, size_t& size)
    {
        if (m_TypeStack.top() == Type::Object) {
            auto& top = m_ObjectStack.top();
            VERIFY(top.is_object());
            size = top.size();

            if (!top.contains(name)) {
                return false;
            }
            m_ObjectStack.push(top[name]);
        } else {
            auto& top = m_ObjectStack.top();
            VERIFY(top.is_array());
            size = top.size();

            m_ObjectStack.push(top[m_IndexStack.top()++]);
        }

        m_TypeStack.push(Type::Object);
        return true;
    }

    void JsonDeserializer::EndObject()
    {
        m_ObjectStack.pop();
        m_TypeStack.pop();
    }

    void JsonDeserializer::BeginArray(std::string_view name, size_t& size)
    {
        if (m_TypeStack.top() == Type::Object) {
            m_ObjectStack.push(m_ObjectStack.top()[name]);
        } else {
            m_ObjectStack.push(m_ObjectStack.top()[m_IndexStack.top()++]);
        }
        size = m_ObjectStack.top().size();

        m_IndexStack.emplace(0);
        m_TypeStack.push(Type::Array);
    }

    void JsonDeserializer::EndArray()
    {
        m_ObjectStack.pop();
        m_IndexStack.pop();
        m_TypeStack.pop();
    }

    auto JsonDeserializer::ReadKeys() -> std::vector<std::string>
    {
        if (m_ObjectStack.top().is_null())
            return {};
        VERIFY(m_ObjectStack.top().is_object());
        std::vector<std::string> keys {};

        for (auto [k, v] : m_ObjectStack.top().items()) {
            (void)v;
            keys.push_back(k);
        }
        return keys;
    }
}
