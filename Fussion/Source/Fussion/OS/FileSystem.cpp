#include "e5pch.h"
#include "FileSystem.h"
#include "Fussion/Core/Core.h"

#include <fstream>

namespace Fussion
{
    std::string FileSystem::ReadEntireFile(std::filesystem::path const& path)
    {
        EASSERT(std::filesystem::exists(path), "Path {} does not exist", path.string());
        std::ifstream file(path);
        return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    }
}