#pragma once
#include <filesystem>

namespace Engin5
{
    class FileSystem
    {
    public:
        static std::string ReadEntireFile(std::filesystem::path const& path);
    };
}
