#pragma once
#include <source_location>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

class Log {
    LogLevel m_Priority{};
public:
    explicit Log(LogLevel default_level = LogLevel::Info);
    ~Log();
    static Log* DefaultLogger();

    void SetLogLevel(LogLevel level);
    void Write(LogLevel level, std::string_view message, std::source_location loc = std::source_location::current());
};

#define LOG_FATAL(message) Log::DefaultLogger()->Write(LogLevel::Fatal, message)
#define LOG_ERROR(message) Log::DefaultLogger()->Write(LogLevel::Error, message)
#define LOG_WARN(message) Log::DefaultLogger()->Write(LogLevel::Warning, message)
#define LOG_INFO(message) Log::DefaultLogger()->Write(LogLevel::Info, message)
#define LOG_DEBUG(message) Log::DefaultLogger()->Write(LogLevel::Debug, message)

#define LOG_FATALF(fmt, ...) Log::DefaultLogger()->Write(LogLevel::Fatal, std::format(fmt, ##__VA_ARGS__))
#define LOG_ERRORF(fmt, ...) Log::DefaultLogger()->Write(LogLevel::Error, std::format(fmt, ##__VA_ARGS__))
#define LOG_WARNF(fmt, ...) Log::DefaultLogger()->Write(LogLevel::Warning, std::format(fmt, ##__VA_ARGS__))
#define LOG_INFOF(fmt, ...) Log::DefaultLogger()->Write(LogLevel::Info, std::format(fmt, ##__VA_ARGS__))
#define LOG_DEBUGF(fmt, ...) Log::DefaultLogger()->Write(LogLevel::Debug, std::format(fmt, ##__VA_ARGS__))
