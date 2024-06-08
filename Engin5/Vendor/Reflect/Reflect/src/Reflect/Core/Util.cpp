#include "Reflect/Core/Util.h"

namespace Reflect
{
    std::string Util::ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](char c) {
            return static_cast<char>(std::tolower(c));
        });
        return str;
    }

    std::string Util::ValidateTypeName(const std::string& str)
    {
        int len = static_cast<int>(str.length());
        int index = len - 1;
        while (str[index] != ' ' && str[index] != str[0]) {
            --index;
        }

        if (index == 0) {
            return str;
        }
        return str.substr(0, index);
    }

    std::string Util::RemoveNamespaces(const std::string& string)
    {
        std::string str = string;
        size_t namespaceChar = string.find_last_of("::");
        if (namespaceChar != std::string::npos) {
            str = str.substr(namespaceChar + 1);
        }
        return str;
    }

    std::string Util::Reverse(std::string str)
    {
        size_t strSize = str.size() - 1;
        for (size_t i = 0; i < str.size() * 0.5f; ++i) {
            size_t upperBound = strSize - i;
            char temp = str.at(i);
            str.at(i) = str.at(upperBound);
            str.at(upperBound) = temp;
        }
        return str;
    }

    void Util::RemoveChar(std::string& str, const char& c)
    {
        size_t index = str.find(c);
        auto itr = str.begin() + (index != -1 ? index : str.size());
        str.erase(itr);
    }

    void Util::RemoveCharReverse(std::string& str, const char& c)
    {
        size_t index = str.rfind(c);
        auto itr = str.begin() + (index != -1 ? index : str.size());
        str.erase(itr);
    }

    void Util::RemoveCharAll(std::string& str, const char& c)
    {
        std::erase(str, c);
    }

    void Util::RemoveString(std::string& str, const std::string& remove, bool removeFromback)
    {
        size_t index = removeFromback ? str.rfind(remove) : str.find(remove);
        if (index != std::string::npos)
            str.erase(index, remove.size());
    }

    bool Util::StringContains(const std::string& str, const std::vector<char>& chars)
    {
        for (const char& strChar : str) {
            bool found = false;
            for (const char& cChars : chars) {
                if (strChar == cChars) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
        return true;
    }

    std::vector<std::string> Util::SplitString(std::string str, char splitChar)
    {
        std::vector<std::string> strings;
        std::string temp;
        temp.reserve(str.size());

        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == splitChar) {
                strings.push_back(temp);
                temp = "";
            }
            else {
                temp += str[i];
            }
        }
        if (!temp.empty()) {
            strings.push_back(temp);
        }

        return strings;
    }

    std::string Util::EReflectValueTypeToString(EReflectValueType mod)
    {
        switch (mod) {
        case EReflectValueType::Value:
            return "";
        case EReflectValueType::Reference:
            return "&";
        case EReflectValueType::Pointer:
            return "*";
        case EReflectValueType::PointerReference:
            return "*&";
        default:
            break;
        }
        return "";
    }
}