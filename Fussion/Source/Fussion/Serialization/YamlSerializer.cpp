#include "FussionPCH.h"

#include "YamlSerializer.h"

namespace Fussion {
    template<typename T>
    void YamlSerializer::GenericWrite(std::string_view name, T value)
    {
        switch (m_TypeStack.top()) {
        case Type::Object:
            m_Emitter << YAML::Key << std::string(name);
            m_Emitter << YAML::Value << value;
            break;
        case Type::Array:
            m_Emitter << value;
            break;
        }
    }

    void YamlSerializer::Initialize()
    {
        m_Emitter << YAML::BeginMap;
        m_TypeStack.push(Type::Object);
    }

    void YamlSerializer::Write(std::string_view name, s8 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, s16 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, s32 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, s64 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, u8 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, u16 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, u32 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, u64 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, f32 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, f64 value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, bool value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, std::string_view value)
    {
        GenericWrite(name, std::string(value));
    }

    void YamlSerializer::Write(std::string_view name, char const* value)
    {
        GenericWrite(name, value);
    }

    void YamlSerializer::Write(std::string_view name, ISerializable const& object)
    {
        BeginObject(name, 0);
        object.Serialize(*this);
        EndObject();
    }

    void YamlSerializer::BeginObject(std::string_view name, size_t size, SerdeOptions const& options)
    {
        (void)size;
        if (m_TypeStack.top() == Type::Object) {
            m_Emitter << YAML::Key << std::string(name) << YAML::Value;
        }
        if (options.test(SerdeOption::Compact)) {
            m_Emitter << YAML::Flow;
        }
        m_Emitter << YAML::BeginMap;
        m_TypeStack.push(Type::Object);
    }

    void YamlSerializer::EndObject()
    {
        m_Emitter << YAML::EndMap;
        m_TypeStack.pop();
    }

    void YamlSerializer::BeginArray(std::string_view name, size_t size)
    {
        if (m_TypeStack.top() == Type::Object) {
            m_Emitter << YAML::Key << std::string(name) << YAML::Value;
        }
        m_Emitter << YAML::BeginSeq;
        m_TypeStack.push(Type::Array);
    }

    void YamlSerializer::EndArray()
    {
        m_Emitter << YAML::EndSeq;
        m_TypeStack.pop();
    }

    void YamlSerializer::WriteByteArray(std::string_view name, u8 const* ptr, usz size)
    {
    }

    std::string YamlSerializer::ToString()
    {
        m_Emitter << YAML::EndMap;
        m_TypeStack.pop();
        return m_Emitter.c_str();
    }

    template<typename T>
    bool YamlDeserializer::GenericRead(std::string_view name, T& value)
    {
        if (m_Nodes.top().IsMap()) {
            auto& node = m_Nodes.top();
            if (!node[std::string(name)]) {
                return false;
            }
            value = node[std::string(name)].as<T>();
        } else if (m_Nodes.top().IsSequence()) {
            value = m_Nodes.top()[m_IndexStack.top()++].as<T>();
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
            m_Nodes.push(std::move(node));
        } catch (YAML::ParserException const& e) {
            LOG_ERRORF("Yaml exception: {}", e.what());
        }
    }

    void YamlDeserializer::Initialize()
    {
    }

    bool YamlDeserializer::Read(std::string_view name, s8& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, s16& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, s32& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, s64& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, u8& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, u16& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, u32& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, u64& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, f32& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, f64& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, bool& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, std::string& value)
    {
        return GenericRead(name, value);
    }

    bool YamlDeserializer::Read(std::string_view name, ISerializable& object)
    {
        if (size_t size; BeginObject(name, size)) {
            object.Deserialize(*this);
            EndObject();
            return true;
        }
        return false;
    }

    bool YamlDeserializer::BeginObject(std::string_view name, size_t& size)
    {
        if (m_Nodes.top().IsMap()) {
            // TODO: use string_view
            if (!m_Nodes.top()[std::string(name)]) {
                size = 0;
                return false;
            }

            auto node = m_Nodes.top()[std::string(name)];
            VERIFY(node.IsMap());
            size = node.size();

            m_Nodes.push(std::move(node));
        } else if (m_Nodes.top().IsSequence()) {
            auto node = m_Nodes.top()[m_IndexStack.top()++];
            VERIFY(node.IsMap());
            size = node.size();

            m_Nodes.push(std::move(node));
        } else {
            PANIC("Unknown node type");
        }

        return true;
    }

    void YamlDeserializer::EndObject()
    {
        m_Nodes.pop();
    }

    void YamlDeserializer::BeginArray(std::string_view name, size_t& size)
    {
        if (m_Nodes.top().IsMap()) {
            // TODO: use string_view
            m_Nodes.push(m_Nodes.top()[std::string(name)]);
        } else if (m_Nodes.top().IsSequence()) {
            m_Nodes.push(m_Nodes.top()[m_IndexStack.top()++]);
        } else {
            size = 0;
            return;
        }
        VERIFY(m_Nodes.top().IsSequence());

        size = m_Nodes.top().size();
        m_IndexStack.emplace(0);
    }

    void YamlDeserializer::EndArray()
    {
        m_Nodes.pop();
    }

    auto YamlDeserializer::ReadKeys() -> std::vector<std::string>
    {
        if (m_Nodes.top().IsNull())
            return {};
        VERIFY(m_Nodes.top().IsMap());
        std::vector<std::string> keys {};

        for (auto aaa : m_Nodes.top()) {
            keys.push_back(aaa.first.Scalar());
        }
        return keys;
    }
}
