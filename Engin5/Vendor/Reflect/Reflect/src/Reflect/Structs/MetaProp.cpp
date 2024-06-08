#include "Reflect/Structs/MetaProp.h"

namespace Reflect
{
    MetaProp::MetaProp(const char* key, const char* value)
        : m_key(key)
          , m_value(value)
    {
    }

    MetaProp::operator bool() const
    {
        return IsValid();
    }

    bool MetaProp::IsValid() const
    {
        return !m_key.empty()
            && !m_value.empty();
    }
}