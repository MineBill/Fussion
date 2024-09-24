#include "FussionPCH.h"
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
        m_out_file.close();
    }

    Ref<FileSink> FileSink::create(std::filesystem::path const& file_name)
    {
        Ref<FileSink> sink;
        sink.reset(new FileSink(file_name.string()));
        return sink;
    }

    FileSink::FileSink(std::string const& file_name)
    {
        m_out_file.open(file_name);
        VERIFY(m_out_file.is_open());
    }

    void FileSink::write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc)
    {
        static const char* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

        if (level >= m_logger->get_priority()) {
            m_out_file << std::format("{} [FileSink]: {}", prefixes[static_cast<int>(level)], message) << std::endl;
        }
    }
}
