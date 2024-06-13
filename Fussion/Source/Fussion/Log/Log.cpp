#include "e5pch.h"
#include "Log.h"
#include <iostream>
#include <format>

#include "Fussion/Core/Types.h"

const std::map<LogLevel, int> g_ColorCodes {
    {LogLevel::Fatal, 31},
    {LogLevel::Error, 31},
    {LogLevel::Warning, 33},
    {LogLevel::Info, 32},
    {LogLevel::Debug, 37},
};

class ConsoleSink final : public LogSink
{
public:
    void Write(LogLevel level, std::string_view message, std::source_location const& loc) override
    {
        static const char* prefixes[] = {"[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]"};

        if (level >= m_Logger->GetPriority()) {
            std::cout <<  std::format("\033[1;{}m{} [ConsoleSink]: {}\033[0m\n", g_ColorCodes.at(level), prefixes[static_cast<int>(level)], message);
            if (level == LogLevel::Fatal) {
                // Do something special?
            }
        }
    }
};

std::unique_ptr<Log> g_DefaultLogger;

Log *Log::DefaultLogger() {
    if (g_DefaultLogger) {
        return g_DefaultLogger.get();
    }
    g_DefaultLogger = std::make_unique<Log>();

    const auto console_sink = MakeRef<ConsoleSink>();
    g_DefaultLogger->RegisterSink(console_sink);
    return g_DefaultLogger.get();
}

Log::Log(LogLevel default_level): m_Priority(default_level) {

}

Log::~Log() = default;

void Log::SetLogLevel(const LogLevel level)
{
    m_Priority = level;
}

void Log::Write(const LogLevel level, const std::string_view message, std::source_location const& loc) const
{
    for (const auto& sink : m_Sinks) {
        sink->Write(level, message, loc);
    }
}

void Log::RegisterSink(Ref<LogSink> const& sink)
{
    sink->m_Logger = this;
    m_Sinks.push_back(sink);
}
