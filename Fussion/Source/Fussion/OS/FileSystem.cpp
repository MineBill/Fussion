#include "FussionPCH.h"
#include "FileSystem.h"
#include "Fussion/Core/Core.h"

#include <fstream>
#include <iterator>

namespace Fussion {
    auto FileSystem::read_entire_file(fs::path const& path) -> Maybe<std::string>
    {
        if (!exists(path)) {
            return None();
        }

        std::ifstream file(path);
        return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
    }

    auto FileSystem::read_entire_file_binary(fs::path const& path) -> Maybe<std::vector<u8>>
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

    void FileSystem::write_entire_file(fs::path const& path, std::string const& string)
    {
        if (!exists(path.parent_path())) {
            fs::create_directories(path.parent_path());
        }
        std::ofstream file(path);
        file << string;
        file.close();
    }
}
