#include "FussionPCH.h"
#include "JsonSerializer.h"

namespace Fussion {
    void JsonSerializer::initialize()
    {
        m_ObjectStack.emplace();
    }

    template<typename T>
    void JsonSerializer::generic_write(std::string_view name, T value)
    {
        if (m_TypeStack.top() == Type::Array) {
            m_ObjectStack.top()[m_IndexStack.top()++] = value;
        } else {
            m_ObjectStack.top()[name] = value;
        }
    }


    void JsonSerializer::write(std::string_view name, s8 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, s16 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, s32 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, s64 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, u8 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, u16 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, u32 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, u64 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, f32 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, f64 value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, bool value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, std::string_view value)
    {
        generic_write(name, value);
    }

    void JsonSerializer::write(std::string_view name, char const* value)
    {
        write(name, std::string_view(value));
    }

    void JsonSerializer::write(std::string_view name, ISerializable const& object)
    {
        begin_object(name, 0);
        object.serialize(*this);
        end_object();
    }

    void JsonSerializer::begin_object(std::string_view name, size_t size)
    {
        (void)size;
        m_ObjectStack.emplace();
        m_Names.emplace(name);
        m_TypeStack.emplace(Type::Object);
    }

    void JsonSerializer::end_object()
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

    void JsonSerializer::begin_array(std::string_view name, size_t size)
    {
        (void)size;
        m_ObjectStack.emplace();
        m_Names.emplace(name);

        m_IndexStack.emplace(0);
        m_TypeStack.emplace(Type::Array);
    }

    void JsonSerializer::end_array()
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

    std::string JsonSerializer::to_string()
    {
        return m_ObjectStack.top().dump(2);
    }

    // ======================================================
    // === Deserializer =====================================
    // ======================================================

    template<typename T>
    void JsonDeserializer::generic_read(std::string_view name, T& value)
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

    JsonDeserializer JsonDeserializer::from_json_object(nlohmann::json const& json)
    {
        JsonDeserializer ds{};
        ds.m_ObjectStack.emplace(nlohmann::ordered_json(json));
        return ds;
    }

    void JsonDeserializer::initialize() {}

    void JsonDeserializer::read(std::string_view name, s8& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, s16& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, s32& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, s64& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, u8& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, u16& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, u32& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, u64& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, f32& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, f64& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, bool& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, std::string& value)
    {
        generic_read(name, value);
    }

    void JsonDeserializer::read(std::string_view name, ISerializable& object)
    {
        if (size_t size; begin_object(name, size)) {
            object.deserialize(*this);
            end_object();
        }
    }

    bool JsonDeserializer::begin_object(std::string_view name, size_t& size)
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

    void JsonDeserializer::end_object()
    {
        m_ObjectStack.pop();
        m_TypeStack.pop();
    }

    void JsonDeserializer::begin_array(std::string_view name, size_t& size)
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

    void JsonDeserializer::end_array()
    {
        m_ObjectStack.pop();
        m_IndexStack.pop();
        m_TypeStack.pop();
    }

    auto JsonDeserializer::read_keys() -> std::vector<std::string>
    {
        if (m_ObjectStack.top().is_null())
            return {};
        VERIFY(m_ObjectStack.top().is_object());
        std::vector<std::string> keys{};

        for (auto [k, v] : m_ObjectStack.top().items()) {
            (void)v;
            keys.push_back(k);
        }
        return std::move(keys);
    }
}
