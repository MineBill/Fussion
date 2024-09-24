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
        static Ref<FileSink> create(std::filesystem::path const& file_name);

        virtual void write(LogLevel level, std::string_view message, std::source_location const& loc) override;

    private:
        std::ofstream m_out_file{};
    };
}
