#include "e5pch.h"
#include "FileSink.h"

#include "Fussion/Core/Core.h"

// @todo Remove this and stop using *_s functions.
#if defined(OS_LINUX)
#include <cerrno>
int fopen_s(FILE **f, const char *name, const char *mode) {
    int ret = 0;
    assert(f);
    *f = fopen(name, mode);
    if (!*f)
        ret = errno;
    return ret;
}

template<typename... Args>
int fprintf_s(FILE* stream, const char* fmt, Args... args)
{
    return fprintf(stream, fmt, args...);
}

#endif

namespace Fussion {
FileSink::~FileSink()
{
    // fflush(m_OutputFile);
    // fclose(m_OutputFile);
    m_OutFile.close();
}

Ref<FileSink> FileSink::Create(std::filesystem::path const& file_name)
{
    Ref<FileSink> sink;
    sink.reset(new FileSink(file_name.string()));
    return sink;
}

FileSink::FileSink(std::string const& file_name)
{
    // const auto fopen_err = fopen_s(&m_OutputFile, file_name.c_str(), "w+");
    // VERIFY(fopen_err == 0, "Could not open/create log file '{}'", file_name);

    m_OutFile.open(file_name);
    VERIFY(m_OutFile.is_open());
}

void FileSink::Write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc)
{
    static const char* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

    if (level >= m_Logger->GetPriority()) {
        // (void)fprintf_s(m_OutputFile, "%s [FileSink]: %s\n", prefixes[static_cast<int>(level)], message.data());
        // fflush(m_OutputFile);
        m_OutFile << std::format("{} [FileSink]: {}", prefixes[static_cast<int>(level)], message) << std::endl;
    }
}
}
