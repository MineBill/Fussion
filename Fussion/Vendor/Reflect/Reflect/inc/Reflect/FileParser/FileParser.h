#pragma once

#include "Reflect/Core/Defines.h"
#include "Reflect/FileParser/ParserStructs.h"

#include "Reflect/Structs/MetaProp.h"

#include <string>
#include <fstream>
#include <unordered_map>
#include <filesystem>

namespace Reflect
{
	struct ReflectAdditionalOptions;
}

namespace Reflect::Parser
{
    class Cache
    {
    public:
        static void Open(std::filesystem::path path);
        static void Save(std::filesystem::path path);

        static void SaveFile(std::filesystem::directory_entry const& entry);
        static time_t GetEntry(std::filesystem::directory_entry const& entry);
    };

	/// <summary>
	/// Parse a single file. This should extract all the info like functions and variables.
	/// </summary>
	class FileParser
	{
	public:
		REFLECT_API FileParser();
		REFLECT_API ~FileParser();

		REFLECT_API void ParseDirectory(const std::string& directory, const ReflectAdditionalOptions* additionalOptions, FileParserOptions fileParserOptions = { });
		/// @brief Clear all parsed file data.
		REFLECT_API void Clear();
		REFLECT_API void SetIgnoreStrings(const std::vector<std::string>& ignoreStrings);

		REFLECT_API const FileParsedData& GetFileParsedData(size_t index) const { return m_filesParsed.at(index); }
		REFLECT_API const std::vector<FileParsedData>& GetAllFileParsedData() const { return m_filesParsed; }

		REFLECT_API const std::string& GetDirectoryParsed(size_t index) const { return m_directoriesParsed.at(index); }
		REFLECT_API const std::vector<std::string>& GetAllDirectoriesParsed() const { return  m_directoriesParsed; }

	private:
		std::ifstream OpenFile(const std::string& filePath);
		void CloseFile(std::ifstream& file);

		bool CheckExtension(const std::string& filePath, std::vector<const char*> extensions);
        static bool CheckModificationTime(std::filesystem::directory_entry const& path);
		bool CheckIfAutoGeneratedFile(const std::string& filePath);

		FileParsedData LoadFile(std::ifstream& file);

		bool ParseFile(FileParsedData& fileData);
		bool ReflectContainerHeader(FileParsedData& fileData, const std::string& keyword, const EReflectType type);
		void ReflectContainer(FileParsedData& fileData);
		void GetAllCPPIncludes(FileParsedData& fileData);

		std::vector<std::string> FindAllNamespaces(FileParsedData fileData, size_t reflectStart);
		std::string FindPreviousNamespace(FileParsedData& fileData);

		std::vector<std::string> FindAllIfDefines(FileParsedData fileData, size_t reflectStart);

		size_t FindEndOfContainer(const FileParsedData& fileData);
		std::vector<std::string> ReflectFlags(FileParsedData fileData);
		std::vector<MetaProp> ReflectMetaProps(FileParsedData fileData);

		void ResolveNamespaces();
		void LinkAllInheritances();
		void RemoveLookupOnlyContainers();

		char FindNextChar(FileParsedData& fileData, const std::vector<char>& ignoreChars, bool reverse = false);
		char FindNextChar(FileParsedData const& fileData, size_t& cursor, const std::vector<char>& ignoreChars, bool reverse = false);
		char FindNextChar(FileParsedData& fileData, char charToFind, bool reverse = false);

		std::string FindNextWord(FileParsedData& fileData, const std::vector<char>& endChars, bool reverse = false);
		std::string FindNextWord(const FileParsedData& fileData, size_t& cursor, const std::vector<char>& endChars, bool reverse = false);
		bool IsWordReflectKey(std::string_view view);

		void MoveToEndOfScope(FileParsedData& fileData, const char startScopeChar, const char endScopeChar) const;

		bool CheckForTypeAlias(std::string_view view);
		bool CheckForVisibility(std::string_view view);
		bool CheckForConstructor(FileParsedData& fileData, Parser::ReflectContainerData& container, std::string_view view);
		bool CheckForIgnoreWord(FileParsedData& fileData, std::string_view view);
		bool CheckForOperatorFunction(FileParsedData& fileData, std::string_view view);
		bool CheckForComments(FileParsedData& fileData, std::string& line);
		bool CheckForFriends(FileParsedData& fileData, std::string_view view);

		void GetReflectNameAndReflectValueTypeAndReflectModifer(std::string& str, std::string& name, EReflectValueType& valueType, EReflectValueModifier& modifer);

		Parser::ReflectFunctionData GetFunction(FileParsedData& fileData, const std::vector<std::string>& flags);
		ReflectMemberData GetMember(FileParsedData& fileData, const std::vector<std::string>& flags);

		void SkipFunctionBody(FileParsedData& fileData);

		EReflectType CheckForReflectType(FileParsedData& data);

		bool CheckForEndOfFile(FileParsedData& fileData, size_t cursor);
		EReflectValueType CheckForRefOrPtr(std::string_view view);
		EReflectValueModifier CheckForMemberModifers(std::string_view view);

		std::string GetFunctionLine(const FileParsedData& fileData, size_t& endCursor);
		std::vector<Parser::ReflectTypeNameData> ReflectGetFunctionParameters(const FileParsedData& fileData, std::string_view view);

		void CheckStringViewBounds(const FileParsedData& fileData, size_t cursor, std::string_view view);
		size_t CountNumberOfSinceTop(const FileParsedData& fileData, size_t cursorStart, const char& character);

		std::string PrettyString(std::string str);
		ReflectContainerData* FindReflectContainerData(std::string_view containerName);

	private:
		std::vector<FileParsedData> m_filesParsed;
		std::vector<std::string> m_ignoreStrings;
		std::vector<std::string> m_directoriesParsed;
		const ReflectAdditionalOptions* m_options;

		std::unordered_map<std::string, std::vector<std::string>> TypeAliasMap;
	};
}
