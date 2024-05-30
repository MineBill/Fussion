#include "e5pch.h"
#include "Log.h"
#include <iostream>
#include <format>

const std::map<LogLevel, int> g_ColorCodes {
    {LogLevel::Fatal, 31},
    {LogLevel::Error, 31},
    {LogLevel::Warning, 33},
    {LogLevel::Info, 32},
    {LogLevel::Debug, 37},
};

std::unique_ptr<Log> g_DefaultLogger;

Log *Log::DefaultLogger() {
    if (g_DefaultLogger) {
        return g_DefaultLogger.get();
    }
    g_DefaultLogger = std::make_unique<Log>();
    return g_DefaultLogger.get();
}

Log::Log(LogLevel default_level): m_Priority(default_level) {

}

Log::~Log() = default;

void Log::SetLogLevel(LogLevel level) {
    m_Priority = level;
}

void Log::Write(LogLevel level, std::string_view message, std::source_location loc) {
    const char* prefixes[] = {"[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]"};

    if (level >= m_Priority) {
        std::cout <<  std::format("\033[1;{}m{}: {}\033[0m\n", g_ColorCodes.at(level), prefixes[static_cast<int>(level)], message);
        if (level == LogLevel::Fatal) {
            // Do something special?
        }
    }
}

