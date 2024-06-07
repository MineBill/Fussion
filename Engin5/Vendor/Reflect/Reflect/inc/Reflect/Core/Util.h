#pragma once

#include "Reflect/FileParser/FileParserKeyWords.h"
#include "Reflect/Core/Enums.h"

#include <string>
#include <algorithm>
#include <vector>
#include <typeinfo>

namespace Reflect
{
	namespace Util
	{
		std::string ToLower(std::string str);
        std::string ValidateTypeName(const std::string& str);
		void RemoveString(std::string& str, const std::string& remove, bool removeFromback = true);
		void RemoveCharAll(std::string& str, const char& c);
		std::string Reverse(std::string str);
        void RemoveChar(std::string& str, const char& c);
        void RemoveCharReverse(std::string& str, const char& c);
        bool StringContains(const std::string& str, const std::vector<char>& chars);
        std::vector<std::string> SplitString(std::string str, char splitChar);
        std::string EReflectValueTypeToString(EReflectValueType mod);
		std::string RemoveNamespaces(const std::string& string);

		template<typename T>
		std::string GetTypeName()
		{
			std::string name = typeid(T).name();
			for (const std::string& key : Keys::ContainerKeys)
			{
				RemoveString(name, key);
			}
			RemoveString(name, Keys::PointerTypeIdKey);
			RemoveCharAll(name, ' ');
			return name;
		}
		template<typename T>
		std::size_t GetTypeSize()
		{
			return sizeof(T);
		}

		template<typename T>
		std::string GetValueTypeName()
		{
			std::string name = GetTypeName<T>();
			RemoveCharAll(name, Keys::ReferenceKey);
			RemoveCharAll(name, Keys::PointerKey);
			return name;
		}

		template<typename T>
		std::size_t GetValueTypeSize()
		{
			return sizeof(std::remove_reference_t<std::remove_pointer_t<T>>);
		}


        template<typename T>
		std::string GetTypeName(const T& type)
		{
			return ValidateTypeName(GetTypeName<T>());
		}

    }
}
