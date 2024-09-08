#include "FussionPCH.h"
#include "Fussion/GPU/ShaderProcessor.h"
#include "Fussion/OS/FileSystem.h"

namespace Fussion::GPU {
    auto ShaderProcessor::process_file(std::filesystem::path const& path) -> Maybe<std::string>
    {
        auto file = FileSystem::read_entire_file(path);
        if (!file) {
            return None();
        }

        std::stringstream ss;

        std::istringstream iss(*file);
        for (std::string line; std::getline(iss, line);) {
            if (!line.starts_with("#")) {
                ss << line << '\n';
            }
        }
        return ss.str();
    }
}
