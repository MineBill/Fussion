#pragma once
#include "Reflect/Structs/MemberInfo.h"
#include "Reflect/Structs/FunctionInfo.h"

#include <optional>

namespace Reflect
{
    /// @brief
    class REFLECT_API TypeInfo
    {
    public:
        using Constructor = void* (*)();

        TypeInfo();
        TypeInfo(
            Type type,
            void* objectInstance,
            Constructor constructor,
            std::vector<TypeInfo> parentInfos,
            std::vector<MemberInfo> memberInfos,
            std::vector<FunctionInfo> functionInfos,
            std::vector<std::string> flags,
            std::vector<MetaProp> metaProps);
        ~TypeInfo();

        operator bool() const;
        bool IsValid() const;

        TypeId GetTypeId() const;
        Type GetType() const;

        bool IsDerivedFrom(const TypeId& typeId) const;

        template <typename T>
        bool IsDerivedFrom() const
        {
            TypeId typeId = TypeId::MakeTypeId<T>();
            return IsDerivedFrom(typeId);
        }

        void* Construct() const
        {
            auto data = m_constructor();
            return data;
        }

        [[nodiscard]] bool HasFlag(const std::string& flag) const;

        [[nodiscard]] bool HasMetaProp(std::string_view name) const;
        [[nodiscard]] MetaProp GetMetaProp(std::string_view key) const;
        [[nodiscard]] std::vector<MetaProp> const& GetMetaProps() const;

        std::vector<TypeInfo> GetParentInfos() const;

        [[nodiscard]] MemberInfo GetMemberInfo(std::string_view memberName) const;
        [[nodiscard]] std::vector<MemberInfo> GetMemberInfosWithFlag(std::string_view flag) const;
        [[nodiscard]] std::vector<MemberInfo> GetMemberInfosWithFlags(std::vector<std::string> flags) const;
        [[nodiscard]] std::vector<MemberInfo> GetMemberInfos() const;

        [[nodiscard]] FunctionInfo GetFunctionInfo(std::string_view memberName) const;
        [[nodiscard]] std::vector<FunctionInfo> GetFunctionInfosWithFlag(std::string_view flag) const;
        [[nodiscard]] std::vector<FunctionInfo> GetFunctionInfosWithFlags(std::vector<std::string> flags) const;
        [[nodiscard]] std::vector<FunctionInfo> GetFunctionInfos() const;

    private:
        void SetObjectInstance(void* objectInstance);

        Type m_type;
        void* m_objectInstance = nullptr;
        Constructor m_constructor;

        std::vector<TypeInfo> m_parentTypeInfos;
        std::vector<MemberInfo> m_memberInfos;
        std::vector<FunctionInfo> m_functionInfos;
        std::vector<std::string> m_flags;
        std::vector<MetaProp> m_metaProps;

        template <typename>
        friend class GenerateTypeInfoForType;
    };
}