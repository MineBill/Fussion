#pragma once
#include "Fussion/Log/Log.h"
#include <source_location>
#include <stacktrace>

#if defined(_MSC_VER)
#define BUILTIN_TRAP_FUNCTION __debugbreak
#elif defined(__clang__) || defined(__GNUC__)
    #define BUILTIN_TRAP_FUNCTION __builtin_trap
#endif

#define USE_ASSERTIONS
#ifdef USE_ASSERTIONS
#define VERIFY(expr, ...)                                                                               \
        {                                                                                               \
            if (!(expr)) {                                                                              \
                LOG_ERRORF("ASSERTION HIT: {}", #expr);                                                 \
                if constexpr (std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value != 0) {    \
                    LOG_ERRORF("    " __VA_ARGS__);                                                     \
                } else {                                                                                \
                    LOG_ERRORF("    No additional information provided");                               \
            }                                                                                           \
            u32 i = 0;                                                                                  \
            LOG_ERRORF("Stacktrace:");                                                                  \
            for (auto const& entry : std::stacktrace::current()) {                                      \
                    LOG_ERRORF("{:^4} > {}:{}", i++, entry.source_file(), entry.source_line());         \
                }                                                                                       \
                BUILTIN_TRAP_FUNCTION();                                                                \
            }                                                                                           \
        }
#else
    #define EASSERT(expr, ...)
    #define ECORE_ASSERT(expr, ...)
#endif

#define PANIC(...)                                                                           \
    {                                                                                        \
        auto loc = std::source_location::current();                                          \
        LOG_ERRORF("PANIC hit at: {}:{}", loc.file_name(), loc.line());                      \
        if constexpr (std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value != 0) { \
            LOG_ERRORF("    " __VA_ARGS__);                                                  \
        } else {                                                                             \
            LOG_ERRORF("   No additional information provided");                             \
        }                                                                                    \
        BUILTIN_TRAP_FUNCTION();                                                             \
    }

#define CAST(type, expression) static_cast<type>(expression)
#define TRANSMUTE(type, expression) reinterpret_cast<type>(expression)
#define require_results [[nodiscard]]
#define MUSTUSE [[nodiscard]]
#define DEPRECATED(msg) [[deprecated(msg)]]

#define UNIMPLEMENTED PANIC("This code path is unimplemented!")
#define UNREACHABLE PANIC("Reached unreachable code!")

namespace Fussion {
    template<typename F>
    struct Defer {
        explicit Defer(F f) : m_F(f) {}
        ~Defer() { m_F(); }

    private:
        F m_F;
    };

    template<typename F>
    Defer<F> MakeDefer(F f)
    {
        return Defer<F>(f);
    }
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = Fussion::MakeDefer([&](){code;})

#define FSN_CLASS(...)
#define FSN_FIELD(...)
