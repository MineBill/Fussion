#pragma once
#include <Fussion/Log/Log.h>
#include <Fussion/Core/Result.h>

namespace Fussion {

enum class SmallVectorError {
    CapacityExceeded,
};

template<typename T, size_t N, bool EnableLogging = false>
class SmallVector {
public:
    constexpr Result<void, SmallVectorError> PushBack(T&& value)
    {
        if (m_Index >= N) {
            if constexpr (EnableLogging) {
                LOG_ERRORF("Small vector exceeded size of {}", N);
            }
            return SmallVectorError::CapacityExceeded;
        }
        m_Array[m_Index++] = value;
        return {};
    }

    [[nodiscard]]
    constexpr T& PopBack()
    {
        return m_Array[--m_Index];
    }

    [[nodiscard]]
    constexpr T* Data() { return m_Array.data(); }

    [[nodiscard]]
    constexpr size_t Size() const { return m_Index; }

    [[nodiscard]]
    static constexpr size_t Capacity() { return N; }

private:
    size_t m_Index{ 0 };
    std::array<T, N> m_Array{};
};

}
