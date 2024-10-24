﻿#include "FileSink.h"

#include "FussionPCH.h"
#include "Fussion/Core/Core.h"

// @todo Remove this and stop using *_s functions.
#if defined(OS_LINUX)
#    include <cerrno>
int fopen_s(FILE** f, char const* name, char const* mode)
{
    int ret = 0;
    assert(f);
    *f = fopen(name, mode);
    if (!*f)
        ret = errno;
    return ret;
}

template<typename... Args>
int fprintf_s(FILE* stream, char const* fmt, Args... args)
{
    return fprintf(stream, fmt, args...);
}

#endif

namespace Fussion {
    FileSink::~FileSink()
    {
        m_OutStream.close();
    }

    Ref<FileSink> FileSink::Create(std::filesystem::path const& file_name)
    {
        Ref<FileSink> sink;
        sink.reset(new FileSink(file_name.string()));
        return sink;
    }

    FileSink::FileSink(std::string const& file_name)
    {
        m_OutStream.open(file_name);
        VERIFY(m_OutStream.is_open());
    }

    void FileSink::Write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc)
    {
        static char const* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

        if (level >= m_logger->GetPriority()) {
            m_OutStream << std::format("{} [FileSink]: {}", prefixes[static_cast<int>(level)], message) << std::endl;
        }
    }
}
