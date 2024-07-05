#pragma once
#include "Fussion/Core/Types.h"

#include <filesystem>

namespace Fussion
{
    class FileSystem
    {
    public:
        static std::optional<std::string> ReadEntireFile(std::filesystem::path const& path);
        static std::vector<u8> ReadEntireFileBinary(std::filesystem::path const& path);

        static void WriteEntireFile(std::filesystem::path const& path, std::string const& string);
    };
}
