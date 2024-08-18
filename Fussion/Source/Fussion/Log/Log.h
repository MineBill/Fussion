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
    LogLevel Level;
    std::string Message;
    std::source_location Location;
};

class LogSink {
    friend class Log;

public:
    virtual ~LogSink() = default;
    virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) = 0;

protected:
    class Log* m_Logger;
};

class Log {
    LogLevel m_Priority{};

public:
    explicit Log(LogLevel default_level = LogLevel::Info);
    ~Log();

    void SetLogLevel(LogLevel level);
    void Write(LogLevel level, std::string_view message,
        std::source_location const& loc = std::source_location::current());

    void RegisterSink(Ref<LogSink> const& sink);

    LogLevel GetPriority() const { return m_Priority; }

    static Log* DefaultLogger();

private:
    std::mutex m_Mutex{};
    std::vector<Ref<LogSink>> m_Sinks{};
};
}

namespace Fsn = Fussion;

#define LOG_FATAL(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Fatal, message)
#define LOG_ERROR(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Error, message)
#define LOG_WARN(message)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Warning, message)
#define LOG_INFO(message)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Info, message)
#define LOG_DEBUG(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Debug, message)

#define LOG_FATALF(fmt_str, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Fatal, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_ERRORF(fmt_str, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Error, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_WARNF(fmt_str, ...)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Warning, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_INFOF(fmt_str, ...)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Info, fmt::format(fmt_str, ##__VA_ARGS__))
#define LOG_DEBUGF(fmt_str, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Debug, fmt::format(fmt_str, ##__VA_ARGS__))
