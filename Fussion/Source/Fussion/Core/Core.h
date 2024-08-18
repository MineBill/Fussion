#pragma once
#include "Fussion/Log/Log.h"

#include <source_location>
#include <stacktrace>

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
    #define VERIFY(expr, ...)
#endif

#define CAST(type, expression) static_cast<type>(expression)
#define TRANSMUTE(type, expression) reinterpret_cast<type>(expression)

#define require_results [[nodiscard]]
#define MUSTUSE [[nodiscard]]
#define DEPRECATED(msg) [[deprecated(msg)]]

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
    inline void Panic()
    {
        auto st = std::stacktrace::current(1, 1)[0];
        LOG_ERRORF("PANIC hit at: {}:{}", st.source_file(), st.source_line());
        LOG_ERRORF("   No additional information provided");
        BUILTIN_TRAP_FUNCTION();

        std::abort(); // Shut up linters
    }

    template<typename... Args>
    [[noreturn]]
    void Panic(fmt::format_string<Args...> message, Args&&... args)
    {
        auto st = std::stacktrace::current(1, 1)[0];
        LOG_ERRORF("PANIC hit at: {}:{}", st.source_file(), st.source_line());
        LOG_ERRORF(message, std::forward<Args>(args)...);
        BUILTIN_TRAP_FUNCTION();

        std::abort(); // Shut up linters
    }
}

#define PANIC(...) Fussion::Panic(__VA_ARGS__)
#define UNIMPLEMENTED PANIC("This code path is unimplemented!")
#define UNREACHABLE PANIC("Reached unreachable code!")

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = Fussion::MakeDefer([&](){code;})
