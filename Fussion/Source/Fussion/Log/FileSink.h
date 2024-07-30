#pragma once
#include "Log.h"
#include "Fussion/Core/Types.h"

#include <cstdio>
#include <fstream>

namespace Fussion {
class FileSink : public LogSink {
    explicit FileSink(std::string const& file_name);

public:
    FileSink() = default;
    virtual ~FileSink() override;
    static Ref<FileSink> Create(std::filesystem::path const& file_name);

    virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) override;

private:
    FILE* m_OutputFile{};
    std::ofstream m_OutFile{};
};
}
