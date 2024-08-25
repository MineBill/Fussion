#pragma once
#include <Fussion/Core/StringUtils.h>

namespace Fussion {
    class Serializer;
    class Deserializer;

    /// Interface for providing serialization and deserialization capabilities for a class.
    class ISerializable {
    public:
        virtual ~ISerializable() = default;

        /// @brief Implement to provide serialization for a type.
        /// @param ctx The serializer context. Do <b>NOT</b> rename. FSN_SERIALIZE depends on the name.
        virtual void Serialize(Serializer& ctx) const;

        /// @brief Implement to provide deserialization for a type.
        /// @param ctx The deserializer context. Do <b>NOT</b> rename.  FSN_DESERIALIZE depends on the name.
        virtual void Deserialize(Deserializer& ctx);
    };
}

#define FSN_SERIALIZE_MEMBER(field) ctx.Write(StringUtils::Remove(#field, "m_"), this->field)
#define FSN_DESERIALIZE_MEMBER(field) ctx.Read(StringUtils::Remove(#field, "m_"), this->field)
