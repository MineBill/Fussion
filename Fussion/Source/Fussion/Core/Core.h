#pragma once
#include "Fussion/Log/Log.h"

#include <cpptrace/cpptrace.hpp>

#if defined(_MSC_VER)
#define BUILTIN_TRAP_FUNCTION __debugbreak
#elif defined(__clang__) || defined(__GNUC__)
    #define BUILTIN_TRAP_FUNCTION __builtin_trap
#endif

#ifdef USE_ASSERTIONS
#define VERIFY(expr, ...)                                                                               \
        {                                                                                               \
            if (!(expr)) {                                                                              \
                LOG_ERRORF("ASSERTION HIT: {}", #expr);                                                 \
                if constexpr (std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value != 0) {    \
                    LOG_ERRORF("    " __VA_ARGS__);                                                     \
                } else {                                                                                \
                    LOG_ERRORF("    No additional information provided");                               \
                }                                                                                       \
                LOG_ERRORF("{}", cpptrace::generate_trace().to_string(true));                           \
                BUILTIN_TRAP_FUNCTION();                                                                \
            }                                                                                           \
        }
#else
    #define VERIFY(expr, ...)
#endif

#define CAST(type, expression) static_cast<type>(expression)
#define TRANSMUTE(type, expression) reinterpret_cast<type>(expression)

namespace Fussion {
    template<typename F>
    struct Defer {
        explicit Defer(F f) : m_f(f) {}
        ~Defer() { m_f(); }

    private:
        F m_f;
    };

    template<typename F>
    Defer<F> make_defer(F f)
    {
        return Defer<F>(f);
    }

    // Panic functions!
    // Panic is implemented as a function to take advantage of the [[noreturn]] attribute
    // and make compilers/tools shut up about not returning from a function when using
    // the UNREACHABLE macro(which panics with a message).
    // Give this code:
    /*
     * int ConvertFromEnum(EnumType arg) {
     *     switch (arg) {
     *     case EnumType::First:
     *         return ...;
     *     case ...:
     *         return ...;
     *     default:
     *     break;
     *     }
     *     UNREACHABLE;
     * }
     *
     * In this case, compilers will warn about the missing return, but we know that will never
     * happen because BUILTIN_TRAP_FUNCTION will eventually be called and terminate the program.
     * Using [[noreturn]] informs the compiler that the function will never return.
     *
     * Also, stacktrace is used instead of source_location because we can't use the latter in
     * the second Panic function that accepts a parameter pack for formatting.
     */

    [[noreturn]]
    inline void panic()
    {
        auto trace = cpptrace::generate_trace(1, 1);
        auto frame = trace.frames[0];
        LOG_ERRORF("PANIC hit at: {}:{}", frame.filename, frame.line.value_or(0));
        LOG_ERRORF("   No additional information provided");
        BUILTIN_TRAP_FUNCTION();

        std::abort(); // Shut up linters
    }

    template<typename... Args>
    [[noreturn]]
    void panic(fmt::format_string<Args...> message, Args&&... args)
    {
        auto trace = cpptrace::generate_trace(1, 1);
        if (!trace.frames.empty()) {
            auto frame = trace.frames[0];
            LOG_ERRORF("PANIC hit at: {}:{}", frame.filename, frame.line.value_or(0));
        } else {
            LOG_ERRORF("PANIC hit. Unable to generate trace");

        }
        LOG_ERRORF(message, std::forward<Args>(args)...);
        BUILTIN_TRAP_FUNCTION();

        std::abort(); // Shut up linters
    }
}

#define PANIC(...) Fussion::panic(__VA_ARGS__)
#define UNIMPLEMENTED PANIC("This code path is unimplemented!")
#define UNREACHABLE PANIC("Reached unreachable code!")

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = Fussion::make_defer([&](){code;})

#include <string_view>
using namespace std::string_view_literals;
