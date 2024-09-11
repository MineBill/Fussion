#pragma once
#include <Fussion/Log/Log.h>
#include <Fussion/Core/Result.h>
#include <array>

namespace Fussion {
    enum class SmallVectorError {
        CapacityExceeded,
    };

    template<typename T, size_t N, bool EnableLogging = false>
    class SmallVector {
    public:
        constexpr Result<void, SmallVectorError> push_back(T&& value)
        {
            if (m_index >= N) {
                if constexpr (EnableLogging) {
                    LOG_ERRORF("Small vector exceeded size of {}", N);
                }
                return SmallVectorError::CapacityExceeded;
            }
            m_array[m_index++] = value;
            return {};
        }

        [[nodiscard]]
        constexpr T& pop_back()
        {
            return m_array[--m_index];
        }

        [[nodiscard]]
        constexpr T* data() { return m_array.data(); }

        [[nodiscard]]
        constexpr size_t size() const { return m_index; }

        [[nodiscard]]
        static constexpr size_t capacity() { return N; }

    private:
        size_t m_index{ 0 };
        std::array<T, N> m_array{};
    };
}
