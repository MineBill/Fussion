#include "FussionPCH.h"

#include "YamlSerializer.h"

namespace Fussion {
    template<typename T>
    void YamlSerializer::generic_write(std::string_view name, T value)
    {
        switch (m_type_stack.top()) {
        case Type::Object:
            m_emitter << YAML::Key << std::string(name);
            m_emitter << YAML::Value << value;
            break;
        case Type::Array:
            m_emitter << value;
            break;
        }
    }

    void YamlSerializer::Initialize()
    {
        m_emitter << YAML::BeginMap;
        m_type_stack.push(Type::Object);
    }

    void YamlSerializer::write(std::string_view name, s8 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, s16 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, s32 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, s64 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, u8 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, u16 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, u32 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, u64 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, f32 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, f64 value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, bool value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, std::string_view value)
    {
        generic_write(name, std::string(value));
    }

    void YamlSerializer::write(std::string_view name, char const* value)
    {
        generic_write(name, value);
    }

    void YamlSerializer::write(std::string_view name, ISerializable const& object)
    {
        begin_object(name, 0);
        object.serialize(*this);
        end_object();
    }

    void YamlSerializer::begin_object(std::string_view name, size_t size, SerdeOptions const& options)
    {
        (void)size;
        if (m_type_stack.top() == Type::Object) {
            m_emitter << YAML::Key << std::string(name) << YAML::Value;
        }
        if (options.test(SerdeOption::Compact)) {
            m_emitter << YAML::Flow;
        }
        m_emitter << YAML::BeginMap;
        m_type_stack.push(Type::Object);
    }

    void YamlSerializer::end_object()
    {
        m_emitter << YAML::EndMap;
        m_type_stack.pop();
    }

    void YamlSerializer::begin_array(std::string_view name, size_t size)
    {
        (void)size;
        if (m_type_stack.top() == Type::Object) {
            m_emitter << YAML::Key << std::string(name) << YAML::Value;
        }
        m_emitter << YAML::BeginSeq;
        m_type_stack.push(Type::Array);
    }

    void YamlSerializer::end_array()
    {
        m_emitter << YAML::EndSeq;
        m_type_stack.pop();
    }

    void YamlSerializer::write_byte_array(std::string_view name, u8 const* ptr, usz size)
    {
        (void)name;
        (void)ptr;
        (void)size;
    }

    std::string YamlSerializer::to_string()
    {
        m_emitter << YAML::EndMap;
        m_type_stack.pop();
        return m_emitter.c_str();
    }

    template<typename T>
    bool YamlDeserializer::generic_read(std::string_view name, T& value)
    {
        if (m_nodes.top().IsMap()) {
            auto& node = m_nodes.top();
            if (!node[std::string(name)]) {
                return false;
            }
            value = node[std::string(name)].as<T>();
        } else if (m_nodes.top().IsSequence()) {
            value = m_nodes.top()[m_index_stack.top()++].as<T>();
        } else {
            LOG_WARNF("Unknown type");
            return false;
        }
        return true;
    }

    YamlDeserializer::YamlDeserializer(std::string const& data)
    {
        try {
            YAML::Node node = YAML::Load(data.c_str());
            m_nodes.push(std::move(node));
        } catch (YAML::ParserException const& e) {
            LOG_ERRORF("Yaml exception: {}", e.what());
        }
    }

    void YamlDeserializer::initialize()
    {
    }

    bool YamlDeserializer::read(std::string_view name, s8& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, s16& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, s32& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, s64& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, u8& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, u16& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, u32& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, u64& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, f32& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, f64& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, bool& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, std::string& value)
    {
        return generic_read(name, value);
    }

    bool YamlDeserializer::read(std::string_view name, ISerializable& object)
    {
        if (size_t size; begin_object(name, size)) {
            object.deserialize(*this);
            end_object();
            return true;
        }
        return false;
    }

    bool YamlDeserializer::read_byte_array(std::string_view name, u8* ptr, size_t size)
    {
        (void)name;
        (void)ptr;
        (void)size;
        return true;
    }

    bool YamlDeserializer::begin_object(std::string_view name, size_t& size)
    {
        if (m_nodes.top().IsMap()) {
            // TODO: use string_view
            if (!m_nodes.top()[std::string(name)]) {
                size = 0;
                return false;
            }

            auto node = m_nodes.top()[std::string(name)];
            VERIFY(node.IsMap());
            size = node.size();

            m_nodes.push(std::move(node));
        } else if (m_nodes.top().IsSequence()) {
            auto node = m_nodes.top()[m_index_stack.top()++];
            VERIFY(node.IsMap());
            size = node.size();

            m_nodes.push(std::move(node));
        } else {
            PANIC("Unknown node type");
        }

        return true;
    }

    void YamlDeserializer::end_object()
    {
        m_nodes.pop();
    }

    void YamlDeserializer::begin_array(std::string_view name, size_t& size)
    {
        if (m_nodes.top().IsMap()) {
            // TODO: use string_view
            m_nodes.push(m_nodes.top()[std::string(name)]);
        } else if (m_nodes.top().IsSequence()) {
            m_nodes.push(m_nodes.top()[m_index_stack.top()++]);
        } else {
            size = 0;
            return;
        }
        VERIFY(m_nodes.top().IsSequence());

        size = m_nodes.top().size();
        m_index_stack.emplace(0);
    }

    void YamlDeserializer::end_array()
    {
        m_nodes.pop();
        m_index_stack.pop();
    }

    auto YamlDeserializer::keys() -> std::vector<std::string>
    {
        if (m_nodes.top().IsNull())
            return {};
        VERIFY(m_nodes.top().IsMap());
        std::vector<std::string> keys {};

        for (auto aaa : m_nodes.top()) {
            keys.push_back(aaa.first.Scalar());
        }
        return keys;
    }
}
