#pragma once
#include "Fussion/Core/Types.h"
#include <source_location>

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
        const std::source_location& loc = std::source_location::current()) const;

    void RegisterSink(Ref<LogSink> const& sink);

    LogLevel GetPriority() const { return m_Priority; }

    static Log* DefaultLogger();

private:
    std::vector<Ref<LogSink>> m_Sinks{};
};
}

namespace Fsn = Fussion;

#define LOG_FATAL(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Fatal, message)
#define LOG_ERROR(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Error, message)
#define LOG_WARN(message)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Warning, message)
#define LOG_INFO(message)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Info, message)
#define LOG_DEBUG(message) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Debug, message)

#define LOG_FATALF(fmt, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Fatal, std::format(fmt, ##__VA_ARGS__))
#define LOG_ERRORF(fmt, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Error, std::format(fmt, ##__VA_ARGS__))
#define LOG_WARNF(fmt, ...)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Warning, std::format(fmt, ##__VA_ARGS__))
#define LOG_INFOF(fmt, ...)  Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Info, std::format(fmt, ##__VA_ARGS__))
#define LOG_DEBUGF(fmt, ...) Fsn::Log::DefaultLogger()->Write(Fsn::LogLevel::Debug, std::format(fmt, ##__VA_ARGS__))
