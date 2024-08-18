#include "FussionPCH.h"
#include "FileSystem.h"
#include "Fussion/Core/Core.h"

#include <fstream>
#include <iterator>

namespace Fussion {
    std::optional<std::string> FileSystem::ReadEntireFile(std::filesystem::path const& path)
    {
        if (!std::filesystem::exists(path)) {
            return std::nullopt;
        }

        std::ifstream file(path);
        return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    }

    std::vector<u8> FileSystem::ReadEntireFileBinary(std::filesystem::path const& path)
    {
        VERIFY(std::filesystem::exists(path), "Path {} does not exist", path.string());
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<u8> buffer(size);

        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    void FileSystem::WriteEntireFile(std::filesystem::path const& path, std::string const& string)
    {
        std::ofstream file(path);
        file << string;
        file.close();
    }
}
