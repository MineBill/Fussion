#include "Reflect/Structs/Property.h"

#include <unordered_set>

namespace Reflect
{
    Property::Property()
    {
    }

    Property::Property(Type objectType, std::string name, PropertyType propertyType, std::vector<std::string> flags,
                       std::vector<MetaProp> propertyMetas, void* objectInstance)
        : m_objectType(std::move(objectType))
          , m_name(std::move(name))
          , m_objectInstance(objectInstance)
          , m_flags(std::move(flags))
          , m_propertyMetas(std::move(propertyMetas))
          , m_propertyType(propertyType)
    {
    }

    Property::operator bool() const
    {
        return IsValid();
    }

    bool Property::IsValid() const
    {
        return m_objectType.IsValid()
            && !m_name.empty()
            && m_objectInstance != nullptr;
    }

    Type Property::GetType() const
    {
        return m_objectType;
    }

    TypeId Property::GetTypeId() const
    {
        return m_objectType.GetTypeId();
    }

    std::string_view Property::GetName() const
    {
        return m_name;
    }

    bool Property::HasFlag(std::string_view flag) const
    {
        for (const std::string& str : m_flags) {
            if (str == flag) {
                return true;
            }
        }
        return false;
    }

    bool Property::HasAnyFlags(const std::vector<std::string>& flags) const
    {
        std::unordered_set<std::string_view> flagsSet;
        for (const std::string& memberFlag : m_flags) {
            flagsSet.insert(memberFlag);
        }

        for (const std::string& flag : flags) {
            if (flagsSet.contains(flag)) {
                return true;
            }
        }
        return false;
    }

    const std::vector<std::string>& Property::GetFlags() const
    {
        return m_flags;
    }

    PropertyType Property::GetPropertyType() const
    {
        return m_propertyType;
    }

    MetaProp Property::GetMeta(std::string_view propertyName) const
    {
        for (size_t i = 0; i < m_propertyMetas.size(); ++i) {
            const MetaProp& meta = m_propertyMetas[i];
            if (meta.GetKey() == propertyName) {
                return meta;
            }
        }
        return MetaProp("", "");
    }

    const std::vector<MetaProp>& Property::GetAllMetas() const
    {
        return m_propertyMetas;
    }
}