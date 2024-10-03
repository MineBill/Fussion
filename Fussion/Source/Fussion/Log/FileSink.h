#pragma once
#include "Fussion/Core/Types.h"
#include "Log.h"

#include <cstdio>
#include <fstream>

namespace Fussion {
    class FileSink final : public LogSink {
        explicit FileSink(std::string const& file_name);

    public:
        FileSink() = default;
        virtual ~FileSink() override;
        static Ref<FileSink> Create(std::filesystem::path const& file_name);

        virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) override;

    private:
        std::ofstream m_OutStream {};
    };
}
