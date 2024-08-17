#pragma once
#include <mutex>

namespace Fussion {
    template<typename T>
    class ThreadProtected {
    public:
        ThreadProtected() = default;

        ThreadProtected(T const& value): m_Object(value) {}
        ThreadProtected(T&& value): m_Object(std::move(value)) {}

        template<typename Func>
        auto Access(Func&& func) const
        {
            using ResultType = std::invoke_result_t<Func, T const&>;
            std::lock_guard lock(m_Mutex);

            if constexpr (std::is_void_v<ResultType>) {
                func(m_Object);
            } else {
                return func(m_Object);
            }
        }

        template<typename Func>
        auto Access(Func&& func)
        {
            using ResultType = std::invoke_result_t<Func, T&>;
            std::lock_guard lock(m_Mutex);

            if constexpr (std::is_void_v<ResultType>) {
                func(m_Object);
            } else {
                return func(m_Object);
            }
        }

        T* UnsafePtr() { return &m_Object; }

    private:
        T m_Object;
        mutable std::mutex m_Mutex;
    };
}
