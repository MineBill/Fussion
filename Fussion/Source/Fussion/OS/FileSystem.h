#pragma once
#include "Fussion/Core/Types.h"
#include <Fussion/Core/Maybe.h>

#include <filesystem>

namespace Fussion
{
    class FileSystem
    {
    public:
        static auto ReadEntireFile(std::filesystem::path const& path) -> Maybe<std::string>;
        static auto ReadEntireFileBinary(std::filesystem::path const& path) -> std::vector<u8>;

        static void WriteEntireFile(std::filesystem::path const& path, std::string const& string);
    };
}
