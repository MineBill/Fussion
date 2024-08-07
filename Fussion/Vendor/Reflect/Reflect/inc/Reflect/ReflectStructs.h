#pragma once

#if REFLECT_TYPE_INFO_ENABLED
#include "Reflect/Core/Defines.h"
#else
#include "Reflect/Core/Enums.h"
#include "Reflect/Core/Util.h"

#include <vector>
#include <memory>
struct ReflectFunction;
struct ReflectMember;
#endif
#include "Structs/TypeId.h"

namespace Reflect
{
#if REFLECT_TYPE_INFO_ENABLED
    template <typename T>
    class GenerateTypeInfoForType;

    struct REFLECT_API IReflect
    {
        const TypeId& GetTypeID()
        {
            return m_TypeId;
        }

    protected:
        TypeId m_TypeId;
    };
#else
	struct ReflectType
	{
		bool operator!=(const ReflectType& other)
		{
			return m_typeName != other.m_typeName ||
				m_typeSize != other.m_typeSize;
		}
		bool operator==(const ReflectType& other)
		{
			return !(*this != other);
		}

		std::size_t GetTypeSize() const { return m_typeSize; }
		std::size_t GetValueTypeSize() const { return m_valueTypeSize; }

		std::string GetTypeName() const { return m_typeName; }
		std::string GetTypeNameWithNamespace() const { return m_typeNameWithNamespace; }

		std::string GetValueTypeName() const { return m_valueTypeName; }
		std::string GetValueTypeNameWithNamespace() const { return m_valueTypeNameWithNamespace; }

		EReflectValueType GetValueType() const { return m_valueType; }

		std::vector<ReflectType*> GetInheritances() const;

		bool IsClass() const;
		bool IsStruct() const;
		bool IsMember() const;
		bool IsFunction() const;
		bool IsParameter() const;

		/// @brief Check if this type is a derivative of T.
		/// @tparam T
		/// @return bool
		template<typename T>
		bool IsDerivedFrom() const
		{
			ReflectTypeCPP<T> base;
			for (const std::unique_ptr<ReflectType>& type : m_inheritanceTypes)
			{
				if (*type.get() == base)
				{
					return true;
				}
			}
			return false;
		}

		virtual void ClearValue(void* data) const = 0;
		virtual void Copy(void* src, void* dst) const = 0;
		virtual void Copy_s(void* src, void* dst, size_t dst_size) const = 0;

	protected:
		ReflectTypeId m_typeId;

		std::size_t m_typeSize;
		std::size_t m_valueTypeSize;

		std::string m_typeName;
		std::string m_typeNameWithNamespace;
		// Store the value type (ex. int* would be int).
		std::string m_valueTypeName;
		std::string m_valueTypeNameWithNamespace;

		EReflectValueType m_typeValue = EReflectValueType::Unknown;
		EReflectValueModifier m_modifier = EReflectValueModifier::None;

		EReflectType m_eReflectType;
		EReflectValueType m_valueType = EReflectValueType::Unknown;

		std::vector<std::unique_ptr<ReflectType>> m_inheritanceTypes;
	};

	template<typename Type>
	struct ReflectTypeCPP : public ReflectType
	{
		using value_type = std::remove_pointer_t<std::remove_reference_t<Type>>;
		ReflectTypeCPP()
		{
			m_typeSize = Util::GetTypeSize<Type>();
			m_valueTypeSize = Util::GetValueTypeSize<Type>();

			m_typeNameWithNamespace = Util::GetTypeName<Type>();
			m_typeName = Util::RemoveNamespaces(m_typeNameWithNamespace);
			m_valueTypeNameWithNamespace = Util::GetValueTypeName<Type>();
			m_valueTypeName = Util::RemoveNamespaces(m_valueTypeNameWithNamespace);
		}

		ReflectTypeCPP(EReflectType eType, EReflectValueType valueType, std::vector<std::unique_ptr<ReflectType>> inheritance, std::string givenName = "")
		{
			m_typeSize = Util::GetTypeSize<Type>();
			m_valueTypeSize = Util::GetValueTypeSize<Type>();

			m_typeNameWithNamespace = Util::GetTypeName<Type>();
			m_typeName = Util::RemoveNamespaces(m_typeNameWithNamespace);
			m_valueTypeNameWithNamespace = Util::GetValueTypeName<Type>();
			m_valueTypeName = Util::RemoveNamespaces(m_valueTypeNameWithNamespace);

			m_eReflectType = eType;
			m_valueType = valueType;

			m_inheritanceTypes = std::move(inheritance);
		}

		virtual void ClearValue(void* data) const override
		{
			if constexpr (!std::is_default_constructible_v<value_type>)
			{
				memset(data, 0, sizeof(value_type));
			}
			else
			{
				Type* t = static_cast<Type*>(data);
				*t = Type();
			}
		}

		virtual void Copy(void* src, void* dst) const override
		{
			if (std::is_pointer_v<Type>)
			{
				value_type* from = static_cast<value_type*>(src);
				value_type* to = static_cast<value_type*>(dst);
				// TODO: Not perfect, but should do for now.
				memcpy(to, from, sizeof(value_type*));
			}
			else
			{
				Type* from = static_cast<Type*>(src);
				Type* to = static_cast<Type*>(dst);
				*to = *from;
			}
		}

		/// <summary>
		/// Safe call for copy. Check the 'value_type' size against dst_size.
		/// </summary>
		/// <param name="src"></param>
		/// <param name="dst"></param>
		/// <param name="dst_size"></param>
		virtual void Copy_s(void* src, void* dst, size_t dst_size) const override
		{
			if (!src || !dst)
			{
				std::cerr << "[ReflectTypeCPP::Copy_s] src: '" << src << "' or dst: '" << dst << "' are not valid.";
			}
			else if (sizeof(value_type) != dst_size)
			{
				std::cerr << "[ReflectTypeCPP::Copy_s] dst_size: '" << dst_size << "' does not match value_type size : '" << sizeof(value_type) << "'.";
			}
			else
			{
				Copy(src, dst);
			}
		}
	};

	template<>
	struct ReflectTypeCPP<void> : public ReflectType
	{
		using value_type = std::remove_pointer_t<std::remove_reference_t<void>>;

		ReflectTypeCPP(EReflectType eType, EReflectValueType valueType, std::vector<std::unique_ptr<ReflectType>> inheritance, std::string givenName = "")
		{
			m_typeName = "void";
			m_typeSize = 0;

			m_valueTypeName = "void";
			m_valueTypeSize = 0;

			m_eReflectType = eType;
			m_valueType = EReflectValueType::Unknown;

			m_inheritanceTypes = std::move(inheritance);
		}

		virtual void ClearValue(void* data) const override
		{ }

		virtual void Copy(void* src, void* dst) const override
		{ }

		/// <summary>
		/// Safe call for copy. Check the 'value_type' size against dst_size.
		/// </summary>
		/// <param name="src"></param>
		/// <param name="dst"></param>
		/// <param name="dst_size"></param>
		virtual void Copy_s(void* src, void* dst, size_t dst_size) const override
		{ }
	};

	using FunctionPtr = Reflect::EReflectReturnCode(*)(void* objectPtr, void* returnValue, FunctionPtrArgs& args);

	class REFLECT_API ReflectTypeInfo;

	/// @brief Describe a single member variable within a class/struct.
	class REFLECT_API ReflectTypeMember
	{
	public:
		ReflectTypeMember(void* ownerClass, const char* name, void* memberPtr, std::unique_ptr<ReflectType> type,
			std::vector<std::string> flags);

		ReflectType* GetType() const;
		std::string_view GetName() const;
		bool IsValid() const;
		bool HasFlag(const char* flag) const;
		void* GetData() const;

		template<typename T>
		T* ConvertToType()
		{
			std::string convertType = Reflect::Util::GetTypeName<T>();
			if (!IsValid() || convertType != m_type->GetTypeName())
			{
				return nullptr;
			}
			return static_cast<T*>(m_memberPtr);
		}

		template<typename T>
		void ModifyValue(T value)
		{
			T* ptr = ConvertToType<T>();
			if (!ptr)
			{
				return;
			}
			*ptr = value;
		}

	private:
		void* m_ownerClass = nullptr;
		void* m_memberPtr = nullptr;
		std::string m_name;
		std::unique_ptr<ReflectType> m_type;
		std::vector<std::string> m_flags;

		friend class ReflectTypeInfo;
	};

	/// @brief Describe a a single function within a class/struct.
	class REFLECT_API ReflectTypeFunction
	{
	public:
		ReflectTypeFunction(void* ownerClass, const char* name, FunctionPtr funcPtr
			, std::unique_ptr<ReflectType> info, std::vector<std::unique_ptr<ReflectType>> args);

		Reflect::EReflectReturnCode Invoke(FunctionPtrArgs functionArgs = FunctionPtrArgs());
		//TODO: FunctionPtr returnValue needs to be a pointer to a pointer. This will allow pointers to be returned from functions.
		template<typename T>
		Reflect::EReflectReturnCode Invoke(T* returnValue, FunctionPtrArgs functionArgs = FunctionPtrArgs())
		{
			return CallInternal((void*)returnValue
				, std::move(functionArgs)
				, ReflectTypeCPP<T>(EReflectType::Unknown, EReflectValueType::Unknown, std::vector<std::unique_ptr<ReflectType>>()));
		}

		bool IsValid() const;

		ReflectType* GetInfo() const;
		std::string_view GetName() const;
		const ReflectType* GetArgInfo(int index) const;
		std::vector<ReflectType*> GetArgsInfo() const;

	protected:
		EReflectReturnCode CallInternal(void* returnValue, FunctionPtrArgs functionArgs, const ReflectType& returnType);

	private:
		bool VerifyOwnerObject() const;
		bool VerifyFunctionPointer() const;
		bool VerifyArgs(const FunctionPtrArgs& functionArgs) const;
		bool CheckReturnType(const ReflectType& returnType) const;

	protected:
		std::unique_ptr<ReflectType> m_info;
		std::string m_name;

		void* m_ownerClass = nullptr;
		FunctionPtr m_func = nullptr;

		int m_numOfArgs;
		std::vector<std::unique_ptr<ReflectType>> m_argsInfo;

		friend class ReflectTypeInfo;
	};

	/// @brief Describe from a high level the class/struct. This should contain basic info such as object name, size,
	class REFLECT_API ReflectTypeInfo
	{
		using ConstructFunc = void* (*)();

	public:
		ReflectTypeInfo(ReflectTypeId typeId
			, void* owner_class
			, std::unique_ptr<ReflectType> info
			, std::vector<std::unique_ptr<ReflectTypeInfo>> inheritances
			, std::vector<std::unique_ptr<ReflectTypeMember>> members
			, std::vector<std::unique_ptr<ReflectTypeFunction>> functions);

		ReflectTypeInfo(const ReflectTypeInfo& other) = delete;
		ReflectTypeInfo(ReflectTypeInfo&& other) noexcept
		{
			(*this) = std::move(other);
		}

		ReflectTypeInfo& operator=(const ReflectTypeInfo& other) = delete;
		ReflectTypeInfo& operator=(ReflectTypeInfo&& other) noexcept
		{
			m_owner_class = std::move(other.m_owner_class);
			m_construct_func = std::move(other.m_construct_func);
			m_info = std::move(other.m_info);
			m_inheritances = std::move(other.m_inheritances);
			m_members = std::move(other.m_members);
			m_functions = std::move(other.m_functions);

			other.m_owner_class = nullptr;
			other.m_construct_func = nullptr;
			other.m_info.reset();
			other.m_inheritances.clear();
			other.m_members.clear();
			other.m_functions.clear();

			return *this;
		}

		ReflectTypeId GetTypeId() const;
		/// @brief Return type info.
		/// @return ReflectType*
		ReflectType* GetInfo() const;

		/// @brief Return weather or not this instance of 'ReflectTypeInfo' has an
		/// owner class/struct tied to it.
		/// @return bool
		bool HasOwner() const;

		/// @brief Construct a new object. Creates a heap allocated object with 'new'. Reflect does not manage the lifetime of this object.
		/// @return void*.
		void* ConstructNew() { if (m_construct_func) { return m_construct_func(); } return nullptr; }

		/// @brief Return a ReflectTypeMember* if found.
		/// @return ReflectTypeMember*
		ReflectTypeMember* GetMember(const char* memberName) const;
		ReflectTypeMember* GetMember(const char* memberName, bool includeBaseClasses) const;

		/// @brief Return all members.
		/// @return std::vector<ReflectTypeMember*>.
		std::vector<ReflectTypeMember*> GetAllMembers() const;
		std::vector<ReflectTypeMember*> GetAllMembers(bool includeBaseClasses) const;

		/// @brief Return all members with flags given.
		/// @return std::vector<ReflectTypeMember*>.
		std::vector<ReflectTypeMember*> GetAllMembersWithFlags(std::vector<const char*> flags) const;
		std::vector<ReflectTypeMember*> GetAllMembersWithFlags(std::vector<const char*> flags, bool includeBaseClasses) const;

		/// @brief Return a ReflectTypeFunction* if found.
		/// @return ReflectTypeFunction*.
		ReflectTypeFunction* GetFunction(const char* functionName) const;
		ReflectTypeFunction* GetFunction(const char* functionName, bool includeBaseClasses) const;

	private:
		ReflectTypeId m_typeId;

		/// @breif Pointer to an instance of which this TypeInfo is made from. This will be nullptr if no instance pointer is given.
		void* m_owner_class = nullptr;

		/// @brief Construct a new object. This uses C++ 'new' so will have to be manually deleted. This is
		/// not done for you.
		ConstructFunc m_construct_func = nullptr;

		/// @brief Base info of this class/struct type.
		std::unique_ptr<ReflectType> m_info;
		/// @brief All class/structs this class/struct inheritances from.
		std::vector<std::unique_ptr<ReflectTypeInfo>> m_inheritances;
		/// @brief Store all the members for this type.
		std::vector<std::unique_ptr<ReflectTypeMember>> m_members;
		/// @brief Store all the functions for this type.
		std::vector<std::unique_ptr<ReflectTypeFunction>> m_functions;

		template<typename>
		friend class GenerateTypeInfoForType;
	};

	/// @brief Template for generating a type's info. Must have a specialisation for each type.
	template<typename T>
	class GenerateTypeInfoForType
	{
	public:
		ReflectTypeInfo GetTypeInfo(T* ownerClass)
		{
			assert(false && "[GenerateTypeInfoForType] This must have a template specialisation.");
			return ReflectTypeInfo(ownerClass, std::make_unique<ReflectTypeCPP<T>>(), {});
		}
	};

	/// @brief Store all registered reflect type info structs.
	class REFLECT_API ReflectTypeInfoRegisty
	{
	public:
		using CreateTypeInfoFunc = ReflectTypeInfo(*)(void* objectInstance);

		ReflectTypeInfoRegisty();
		~ReflectTypeInfoRegisty();

		static void RegisterTypeInfo(ReflectTypeId typeId, CreateTypeInfoFunc createTypeInfoFunc);
		static void UnregisterTypeInfo(ReflectTypeId typeId);

		static ReflectTypeInfo GetTypeInfo(const ReflectTypeId& typeId);
		static ReflectTypeInfo GetTypeInfo(const ReflectTypeId& typeId, void* objectInstance);
		template<typename T>
		static ReflectTypeInfo GetTypeInfo()
		{
			static_assert(std::is_base_of_v<IReflect, T>);
			return GetTypeInfo(T::GetReflectTypeId());
		}

	//private:
		static ReflectTypeInfoRegisty& Instance()
		{
			static ReflectTypeInfoRegisty instance;
			return instance;
		}

	private:
		std::mutex m_registyLock;
		std::unordered_map<ReflectTypeId, CreateTypeInfoFunc> m_registy;
	};

	/// @brief Allow for a type to be registered when the constructor is called,
	/// and un registered when the destructor is called.
	struct REFLECT_API ReflectTypeInfoRegister
	{
		ReflectTypeId m_typeId;
		ReflectTypeInfoRegister(){ }

		ReflectTypeInfoRegister(const char* typeName, ReflectTypeInfoRegisty::CreateTypeInfoFunc createTypeInfoFunc)
		{
			m_typeId = ReflectTypeId(typeName);
			ReflectTypeInfoRegisty::RegisterTypeInfo(m_typeId, createTypeInfoFunc);
		}

		~ReflectTypeInfoRegister()
		{
			ReflectTypeInfoRegisty::UnregisterTypeInfo(m_typeId);
		}
	};

	struct ReflectMemberProp
	{
		ReflectMemberProp(const char* name, ReflectType* typeCPP, size_t offset, std::vector<std::string> const& strProperties)
			: Name(name)
			, Type(typeCPP)
			, Offset(offset)
			, StrProperties(strProperties)
		{ }

		~ReflectMemberProp()
		{
			delete Type;
		}

		bool ContainsProperty(std::vector<std::string> const& flags)
		{
			for (auto const& flag : flags)
			{
				for (auto const& p : StrProperties)
				{
					if (p == flag)
					{
						return true;
					}
				}
			}
			return false;
		}

		const char* Name;
		ReflectType* Type;
		size_t Offset;
		std::vector<std::string> StrProperties;
	};

	struct ReflectFunction
	{
		ReflectFunction(void* objectPtr, FunctionPtr func)
			: m_objectPtr(objectPtr)
			, m_func(func)
		{ }

		//template<typename... Args>
		//void Invoke(void* returnValue, Args... args)
		//{
		//	FunctionPtrArgs funcArgs = PackFunctionArgs(std::forward<Args>(args)...);
		//	int i = *static_cast<int*>(funcArgs.GetArg(0));
		//	int* ip = static_cast<int*>(funcArgs.GetArg(1));
		//	return (*Func)(ObjectPtr, returnValue, funcArgs);
		//}

		//TODO: FunctionPtr returnValuePointer needs to be a pointer to a pointer. This will allow pointers to be returned from functions.
		template<typename T>
		Reflect::EReflectReturnCode Invoke(T* returnValue, FunctionPtrArgs functionArgs = FunctionPtrArgs())
		{
			return Invoke((void*)returnValue, std::move(functionArgs));
		}

		Reflect::EReflectReturnCode Invoke(void* returnValue = nullptr, FunctionPtrArgs functionArgs = FunctionPtrArgs())
		{
			if (IsValid())
			{
				(*m_func)(m_objectPtr, returnValue, functionArgs);
				return EReflectReturnCode::SUCCESS;
			}
			return EReflectReturnCode::INVALID_FUNCTION_POINTER;
		}

		bool IsValid() const
		{
			return m_objectPtr != nullptr;
		}

	private:
		template<typename... Args>
		FunctionPtrArgs PackFunctionArgs(Args... args)
		{
			std::vector<FunctionPtrArgs::Arg> funcArgs = { PackFunctionArg(args)... };
			return  FunctionPtrArgs(funcArgs);
		}

		template<typename T, typename... Args>
		FunctionPtrArgs::Arg PackFunctionArg(T& t, Args&... args)
		{
			return FunctionPtrArgs::Arg(Reflect::Util::GetTypeName(t), &t);
		}

		template<typename T, typename... Args>
		FunctionPtrArgs::Arg PackFunctionArg(T* t, Args... args)
		{
			return FunctionPtrArgs::Arg(Reflect::Util::GetTypeName(t), static_cast<void*>(t));
		}

	private:
		void* m_objectPtr;
		FunctionPtr m_func;
	};

	struct ReflectMember
	{
		ReflectMember(const char* memberName, ReflectType* type, void* memberPtr)
			: m_name(memberName)
			, m_type(type)
			, m_ptr(memberPtr)
		{}

		bool IsValid() const
		{
			return m_ptr != nullptr;
		}

		void* GetRawPointer() { return m_ptr; }

		std::string GetName() { return m_name; }

		const ReflectType* GetType() const { return m_type; }

		template<typename T>
		T* ConvertToType()
		{
			std::string convertType = Reflect::Util::GetTypeName<T>();
			if (!IsValid() || convertType != m_type->GetTypeName())
			{
				return nullptr;
			}
			return static_cast<T*>(m_ptr);
		}

	private:
		const char* m_name;
		ReflectType* m_type;
		void* m_ptr;
		int m_offset;
	};
	struct REFLECT_API IReflect
	{
		IReflect() { InitialiseMembers(); }

		virtual ReflectFunction GetFunction(const char* functionName) { (void)functionName; return ReflectFunction(nullptr, nullptr); };
		virtual ReflectMember GetMember(const char* memberName) { (void)memberName; return ReflectMember("", nullptr, nullptr); };
		virtual std::vector<ReflectMember> GetMembers(std::vector<std::string> const& flags) { (void)flags; return {}; };
		virtual std::vector<ReflectMember> GetAllMembers() { return {}; };

	protected:
		virtual void InitialiseMembers() { }
	};
#endif
}

#define REFLECT_BASE public Reflect::IReflect