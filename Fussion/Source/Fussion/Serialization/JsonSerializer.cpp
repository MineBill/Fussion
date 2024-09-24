#include "FussionPCH.h"
#include "JsonSerializer.h"

namespace Fussion {
    void JsonSerializer::initialize()
    {
        m_object_stack.emplace();
    }

    template<typename T>
    void JsonSerializer::generic_write(std::string_view name, T value)
    {
        if (m_type_stack.top() == Type::Array) {
            m_object_stack.top()[m_index_stack.top()++] = value;
        } else {
            m_object_stack.top()[name] = value;
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
        m_object_stack.emplace();
        m_names.emplace(name);
        m_type_stack.emplace(Type::Object);
    }

    void JsonSerializer::end_object()
    {
        auto top = std::move(m_object_stack.top());
        m_object_stack.pop();

        auto name = std::move(m_names.top());
        m_names.pop();

        m_type_stack.pop();

        if (m_type_stack.top() == Type::Array) {
            m_object_stack.top()[m_index_stack.top()++] = top;
        } else {
            m_object_stack.top()[name] = top;
        }
    }

    void JsonSerializer::begin_array(std::string_view name, size_t size)
    {
        (void)size;
        m_object_stack.emplace();
        m_names.emplace(name);

        m_index_stack.emplace(0);
        m_type_stack.emplace(Type::Array);
    }

    void JsonSerializer::end_array()
    {
        auto top = std::move(m_object_stack.top());
        m_object_stack.pop();

        auto name = std::move(m_names.top());
        m_names.pop();

        m_index_stack.pop();
        m_type_stack.pop();

        if (m_type_stack.top() == Type::Array) {
            m_object_stack.top()[m_index_stack.top()++] = top;
        } else {
            m_object_stack.top()[name] = top;
        }
    }

    std::string JsonSerializer::to_string()
    {
        return m_object_stack.top().dump(2);
    }

    // ======================================================
    // === Deserializer =====================================
    // ======================================================

    template<typename T>
    void JsonDeserializer::generic_read(std::string_view name, T& value)
    {
        if (m_object_stack.empty())
            return;
        if (m_type_stack.top() == Type::Array) {
            value = m_object_stack.top().at(m_index_stack.top()++);
        } else {
            value = m_object_stack.top().value(name, value);
        }
    }

    JsonDeserializer::JsonDeserializer(std::string const& data)
    {
        try {
            m_object_stack.emplace(nlohmann::json::parse(data, nullptr, true, true));
        } catch (std::exception const& e) {
            LOG_ERRORF("Caught exception when parsing JSON: {}", e.what());
        }
    }

    JsonDeserializer JsonDeserializer::from_json_object(nlohmann::json const& json)
    {
        JsonDeserializer ds{};
        ds.m_object_stack.emplace(nlohmann::ordered_json(json));
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
        if (m_type_stack.top() == Type::Object) {
            auto& top = m_object_stack.top();
            VERIFY(top.is_object());
            size = top.size();

            if (!top.contains(name)) {
                return false;
            }
            m_object_stack.push(top[name]);
        } else {
            auto& top = m_object_stack.top();
            VERIFY(top.is_array());
            size = top.size();

            m_object_stack.push(top[m_index_stack.top()++]);
        }

        m_type_stack.push(Type::Object);
        return true;
    }

    void JsonDeserializer::end_object()
    {
        m_object_stack.pop();
        m_type_stack.pop();
    }

    void JsonDeserializer::begin_array(std::string_view name, size_t& size)
    {
        if (m_type_stack.top() == Type::Object) {
            m_object_stack.push(m_object_stack.top()[name]);
        } else {
            m_object_stack.push(m_object_stack.top()[m_index_stack.top()++]);
        }
        size = m_object_stack.top().size();

        m_index_stack.emplace(0);
        m_type_stack.push(Type::Array);
    }

    void JsonDeserializer::end_array()
    {
        m_object_stack.pop();
        m_index_stack.pop();
        m_type_stack.pop();
    }

    auto JsonDeserializer::read_keys() -> std::vector<std::string>
    {
        if (m_object_stack.top().is_null())
            return {};
        VERIFY(m_object_stack.top().is_object());
        std::vector<std::string> keys{};

        for (auto [k, v] : m_object_stack.top().items()) {
            (void)v;
            keys.push_back(k);
        }
        return keys;
    }
}
