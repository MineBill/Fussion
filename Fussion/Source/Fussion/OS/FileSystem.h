#pragma once
#include <filesystem>

namespace Fussion
{
    class FileSystem
    {
    public:
        static std::string ReadEntireFile(std::filesystem::path const& path);

        static void WriteEntireFile(std::filesystem::path const& path, std::string const& string);
    };
}
