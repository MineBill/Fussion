#pragma once
#include "Fussion/Core/Types.h"

#include <fmt/format.h>

#include <source_location>
#include <mutex>
#include <vector>

namespace Fussion {
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::source_location location;
};

class LogSink {
    friend class Log;

public:
    virtual ~LogSink() = default;
    virtual void write(LogLevel level, std::string_view message, std::source_location const& loc) = 0;

protected:
    class Log* m_logger;
};

class Log {
    LogLevel m_priority{};

public:
    explicit Log(LogLevel default_level = LogLevel::Info);
    ~Log();

    void set_log_level(LogLevel level);
    void write(LogLevel level, std::string_view message,
        std::source_location const& loc = std::source_location::current());

    void register_sink(Ref<LogSink> const& sink);

    LogLevel get_priority() const { return m_priority; }

    static Log* default_logger();

private:
    std::mutex m_mutex{};
    std::vector<Ref<LogSink>> m_sinks{};
};
}

namespace Fsn = Fussion;

#define LOG_FATAL(message) Fsn::Log::default_logger()->write(Fsn::LogLevel::Fatal, message)
#define LOG_ERROR(message) Fsn::Log::default_logger()->write(Fsn::LogLevel::Error, message)
#define LOG_WARN(message)  Fsn::Log::default_logger()->write(Fsn::LogLevel::Warning, message)
#define LOG_INFO(message)  Fsn::Log::default_logger()->write(Fsn::LogLevel::Info, message)
#define LOG_DEBUG(message) Fsn::Log::default_logger()->write(Fsn::LogLevel::Debug, message)

#define LOG_FATALF(fmt_str, ...) Fsn::Log::default_logger()->write(Fsn::LogLevel::Fatal, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_ERRORF(fmt_str, ...) Fsn::Log::default_logger()->write(Fsn::LogLevel::Error, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_WARNF(fmt_str, ...)  Fsn::Log::default_logger()->write(Fsn::LogLevel::Warning, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_INFOF(fmt_str, ...)  Fsn::Log::default_logger()->write(Fsn::LogLevel::Info, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_DEBUGF(fmt_str, ...) Fsn::Log::default_logger()->write(Fsn::LogLevel::Debug, fmt::format(fmt_str, ##__VA_ARGS__))
