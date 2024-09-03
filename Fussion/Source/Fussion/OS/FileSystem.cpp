#include "FussionPCH.h"
#include "FileSystem.h"
#include "Fussion/Core/Core.h"

#include <fstream>
#include <iterator>

namespace Fussion {
    auto FileSystem::ReadEntireFile(fs::path const& path) -> Maybe<std::string>
    {
        if (!exists(path)) {
            return None();
        }

        std::ifstream file(path);
        return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    }

    auto FileSystem::ReadEntireFileBinary(fs::path const& path) -> Maybe<std::vector<u8>>
    {
        if (!exists(path)) {
            return None();
        }
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<u8> buffer(size);

        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    void FileSystem::WriteEntireFile(fs::path const& path, std::string const& string)
    {
        std::ofstream file(path);
        file << string;
        file.close();
    }
}
