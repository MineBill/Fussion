#include "FussionPCH.h"
#include "Log.h"

#include "Fussion/Core/Types.h"
#include <Fussion/OS/System.h>

#include <iostream>

namespace Fussion {
    class ConsoleSink final : public LogSink {
    public:
        bool EnableColor;
        ConsoleSink()
        {
            EnableColor = System::ConsoleSupportsColor();
        }

        virtual void Write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc) override
        {
            static std::map<LogLevel, int> const ColorCodes {
                { LogLevel::Fatal, 196 },
                { LogLevel::Error, 160 },
                { LogLevel::Warning, 178 },
                { LogLevel::Info, 114 },
                { LogLevel::Debug, 240 },
            };

            static char const* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

            if (level >= m_logger->GetPriority()) {
                switch (level) {
                case LogLevel::Debug:
                case LogLevel::Info:
                case LogLevel::Warning:
                    if (EnableColor) {
                        std::cout << fmt::format("\033[38;5;{}m[Console] {}: {}\033[0m\n", ColorCodes.at(level), prefixes[CAST(int, level)], message);
                    } else {
                        std::cout << fmt::format("[Console] {}: {}\n", prefixes[CAST(int, level)], message);
                    }
                    break;
                case LogLevel::Error:
                case LogLevel::Fatal:
                    if (EnableColor) {
                        std::cerr << fmt::format("\033[38;5;{}m{} [Console]: {}\033[0m\n", ColorCodes.at(level), prefixes[CAST(int, level)], message);
                    } else {
                        std::cerr << fmt::format("[Console] {}: {}\n", prefixes[CAST(int, level)], message);
                    }
                    break;
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

        auto const console_sink = MakeRef<ConsoleSink>();
        g_DefaultLogger->RegisterSink(console_sink);
        return g_DefaultLogger;
    }

    Log::Log(LogLevel default_level)
        : m_Priority(default_level)
    { }

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
        sink->m_logger = this;
        m_Sinks.push_back(sink);
    }

    void Log::RemoveSink(Ref<LogSink> const& sink)
    {
        s32 pos = -1;
        for (usz i = 0; i < m_Sinks.size(); i++) {
            if (m_Sinks[i].get() == sink.get()) {
                pos = CAST(s32, i);
                break;
            }
        }
        if (pos != -1) {
            m_Sinks.erase(m_Sinks.begin() + pos);
        }
    }
}
