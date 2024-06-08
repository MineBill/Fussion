#pragma once

#include "Reflect/Structs/Type.h"
#include "Reflect/Structs/Property.h"

namespace Reflect
{
    class TypeInfo;

    /// @brief A MemberInfo is a representation of a member variable which has been reflected
    /// on a class/struct.
    class REFLECT_API MemberInfo : public Property
    {
    public:
        MemberInfo();
        MemberInfo(Type type, std::string memberName, EReflectValueType valueType,
                   EReflectValueModifier modiferType, std::vector<std::string> flags, uint64_t offset,
                   void* objectInstance);
        MemberInfo(Type type, std::string memberName, EReflectValueType valueType,
                   EReflectValueModifier modiferType, std::vector<std::string> flags,
                   std::vector<MetaProp> propertyMetas,
                   uint64_t offset, void* objectInstance);
        ~MemberInfo();

        EReflectValueType GetValueType() const;
        EReflectValueModifier GetModifierType() const;

        void* GetMemberPointer() const;

        /// @brief Get a pointer to this member.
        /// @tparam T The type to use.
        /// @return A pointer to the member T or nullptr if the member is not of type T.
        template <typename T>
        T* GetMemberPointer() const
        {
            if (!IsType<T>())
                return nullptr;
            void* memberPtr = GetMemberPointer();
            if (!memberPtr) {
                return nullptr;
            }
            return static_cast<T*>(memberPtr);
        }

    private:
        void SetObjectInstance(void* objectInstance);

        /// @brief Is this member of type value, pointer or reference.
        EReflectValueType m_valueType = EReflectValueType::Unknown;
        /// @brief Is this member a const, static or volatile.
        EReflectValueModifier m_modifierType = EReflectValueModifier::None;

        /// @brief Offset of this member in the object.
        uint64_t m_memberOffset = 0;
        friend TypeInfo;
    };
}