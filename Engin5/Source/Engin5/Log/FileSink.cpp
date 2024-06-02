#include "e5pch.h"
#include "FileSink.h"

#include "Engin5/Core/Core.h"

namespace Engin5
{
    FileSink::~FileSink()
    {
        fflush(m_OutputFile);
        fclose(m_OutputFile);
    }

    Ref<FileSink> FileSink::Create(std::string const& file_name)
    {
        Ref<FileSink> sink;
        sink.reset(new FileSink(file_name));
        return sink;
    }

    FileSink::FileSink(std::string const& file_name)
    {
        const auto fopen_err = fopen_s(&m_OutputFile, file_name.c_str(), "w");
        EASSERT(fopen_err == 0, "Could not open/create log file '{}'", file_name);
    }

    void FileSink::Write(LogLevel level, std::string_view message, std::source_location const& loc)
    {
        static const char* prefixes[] = {"[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]"};

        if (level >= m_Logger->GetPriority()) {
            (void)fprintf_s(m_OutputFile, "%s [FileSink]: %s\n", prefixes[static_cast<int>(level)], message.data());
        }
    }
}