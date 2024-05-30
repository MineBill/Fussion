#include "e5pch.h"
#include "FileSystem.h"
#include "Engin5/Core/Core.h"

#include <fstream>

namespace Engin5
{
    std::string FileSystem::ReadEntireFile(std::filesystem::path const& path)
    {
        EASSERT(std::filesystem::exists(path), "Path {} does not exist", path.string());
        std::ifstream file(path);
        return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    }
}