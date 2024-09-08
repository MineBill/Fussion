#pragma once
#include <mutex>

namespace Fussion {
    template<typename T>
    class ThreadProtected {
    public:
        ThreadProtected() = default;

        ThreadProtected(T const& value): m_object(value) {}
        ThreadProtected(T&& value): m_object(std::move(value)) {}

        template<typename Func>
        auto access(Func&& func) const
        {
            using ResultType = std::invoke_result_t<Func, T const&>;
            std::lock_guard lock(m_mutex);

            if constexpr (std::is_void_v<ResultType>) {
                func(m_object);
            } else {
                return func(m_object);
            }
        }

        template<typename Func>
        auto access(Func&& func)
        {
            using ResultType = std::invoke_result_t<Func, T&>;
            std::lock_guard lock(m_mutex);

            if constexpr (std::is_void_v<ResultType>) {
                func(m_object);
            } else {
                return func(m_object);
            }
        }

        T* unsafe_ptr() { return &m_object; }

    private:
        T m_object;
        mutable std::mutex m_mutex;
    };
}
