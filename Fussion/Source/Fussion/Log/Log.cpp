#include "FussionPCH.h"
#include "Log.h"

#include "Fussion/Core/Types.h"

#include <iostream>

namespace Fussion {
    class ConsoleSink final : public LogSink {
    public:
        virtual void write(LogLevel level, std::string_view message, [[maybe_unused]] std::source_location const& loc) override
        {
            static std::map<LogLevel, int> const ColorCodes{
                { LogLevel::Fatal, 196 },
                { LogLevel::Error, 160 },
                { LogLevel::Warning, 178 },
                { LogLevel::Info, 114 },
                { LogLevel::Debug, 240 },
            };

            static const char* prefixes[] = { "[ DEBUG ]", "[ INFO  ]", "[WARNING]", "[ ERROR ]", "[ FATAL ]" };

            if (level >= m_logger->get_priority()) {
                std::cout << fmt::format("\033[38;5;{}m{} [Console]: {}\033[0m\n", ColorCodes.at(level), prefixes[static_cast<int>(level)], message);
                if (level == LogLevel::Fatal) {
                    // Do something special?
                }
            }
        }
    };

    // We specifically make this a raw ptr to ensure it will never be freed.
    Log* g_DefaultLogger;

    Log* Log::default_logger()
    {
        if (g_DefaultLogger) {
            return g_DefaultLogger;
        }
        g_DefaultLogger = new Log();

        const auto console_sink = make_ref<ConsoleSink>();
        g_DefaultLogger->register_sink(console_sink);
        return g_DefaultLogger;
    }

    Log::Log(LogLevel default_level): m_priority(default_level) {}

    Log::~Log() = default;

    void Log::set_log_level(LogLevel level)
    {
        m_priority = level;
    }

    void Log::write(LogLevel level, std::string_view message, std::source_location const& loc)
    {
        std::scoped_lock lock(m_mutex);
        for (auto const& sink : m_sinks) {
            sink->write(level, message, loc);
        }
    }

    void Log::register_sink(Ref<LogSink> const& sink)
    {
        sink->m_logger = this;
        m_sinks.push_back(sink);
    }
}
