#pragma once
#include "Log.h"
#include "Fussion/Core/Types.h"

#include <cstdio>

namespace Fussion
{
    class FileSink: public LogSink
    {
        explicit FileSink(std::string const& file_name);
    public:
        FileSink() = default;
        ~FileSink() override;
        static Ref<FileSink> Create(std::string const& file_name);

        void Write(LogLevel level, std::string_view message, std::source_location const& loc) override;

    private:
        FILE* m_OutputFile{};
    };
}
