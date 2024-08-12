#include "e5pch.h"
#include "Log.h"
#include <iostream>
#include <format>

#include "Fussion/Core/Types.h"

namespace Fussion {
    class ConsoleSink final : public LogSink {
    public:
        virtual void Write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc) override
        {
            static std::map<LogLevel, int> const ColorCodes{
                { LogLevel::Fatal, 196 },
                { LogLevel::Error, 160 },
                { LogLevel::Warning, 178 },
                { LogLevel::Info, 114 },
                { LogLevel::Debug, 240 },
            };

            static const char* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

            if (level >= m_Logger->GetPriority()) {
                std::cout << std::format("\033[38;5;{}m{} [Console]: {}\033[0m\n", ColorCodes.at(level), prefixes[static_cast<int>(level)], message);
                if (level == LogLevel::Fatal) {
                    // Do something special?
                }
            }
        }
    };

    // We specifically make this a raw ptr to ensure it will never be freed.
    Log* g_DefaultLogger;

    Log* Log::DefaultLogger()
    {
        if (g_DefaultLogger) {
            return g_DefaultLogger;
        }
        g_DefaultLogger = new Log();

        const auto console_sink = MakeRef<ConsoleSink>();
        g_DefaultLogger->RegisterSink(console_sink);
        return g_DefaultLogger;
    }

    Log::Log(LogLevel default_level): m_Priority(default_level) {}

    Log::~Log() = default;

    void Log::SetLogLevel(LogLevel level)
    {
        m_Priority = level;
    }

    void Log::Write(LogLevel level, std::string_view message, std::source_location const& loc)
    {
        std::scoped_lock lock(m_Mutex);
        for (auto const& sink : m_Sinks) {
            sink->Write(level, message, loc);
        }
    }

    void Log::RegisterSink(Ref<LogSink> const& sink)
    {
        sink->m_Logger = this;
        m_Sinks.push_back(sink);
    }
}
