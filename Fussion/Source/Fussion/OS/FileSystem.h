#pragma once
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Types.h>

#include <filesystem>

namespace Fussion {
    namespace fs = std::filesystem;

    class FileSystem {
    public:
        /// Read entire file as a string.
        /// @return The string or an empty if the file does not exist.
        static auto ReadEntireFile(fs::path const& path) -> Maybe<std::string>;
        /// Read entire file in binary as a vector of bytes.
        /// @return The vector or an empty if the file does not exist.
        static auto ReadEntireFileBinary(fs::path const& path) -> Maybe<std::vector<u8>>;

        static void WriteEntireFile(fs::path const& path, std::string const& string);
    };
}
