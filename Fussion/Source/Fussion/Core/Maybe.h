#pragma once
#include "Core.h"

#include <type_traits>

namespace Fussion {
    template<typename T>
    class Maybe {
    public:
        constexpr Maybe(): m_NullableValue(nullptr) {}
        constexpr ~Maybe() { Reset(); }

        constexpr Maybe(T const& _value):
            m_NullableValue(new(m_Storage) T(_value)) {}

        constexpr Maybe(T&& _value):
            m_NullableValue(new(m_Storage) T(std::move(_value))) {}

        constexpr Maybe& operator=(T const& _value)
        {
            Reset();
            m_NullableValue = new(m_Storage) T(_value);
            return *this;
        }

        constexpr Maybe& operator=(T&& _value)
        {
            Reset();
            m_NullableValue = new(m_Storage) T(std::move(_value));
            return *this;
        }

        constexpr Maybe(Maybe const& _other):
            m_NullableValue(_other ? new(m_Storage) T(*_other) : nullptr) {}

        constexpr Maybe(Maybe&& _other) noexcept:
            m_NullableValue(_other
                ? new(m_Storage) T(std::move(*_other))
                : nullptr) {}

        constexpr Maybe& operator=(Maybe const& _other)
        {
            if (&_other != this) {
                Reset();
                if (_other) { m_NullableValue = new(m_Storage) T(*_other); }
            }
            return *this;
        }

        constexpr Maybe& operator=(Maybe&& _other) noexcept
        {
            if (&_other != this) {
                Reset();
                if (_other) {
                    m_NullableValue = new(m_Storage) T(std::move(*_other));
                }
            }
            return *this;
        }

        constexpr void Reset()
        {
            if (m_NullableValue) { m_NullableValue->~T(); }
            m_NullableValue = nullptr;
        }

        constexpr T& operator*()
        {
            VERIFY(m_NullableValue);
            return *m_NullableValue;
        }

        constexpr T const& operator*() const
        {
            VERIFY(m_NullableValue);
            return *m_NullableValue;
        }

        constexpr T* operator->()
        {
            VERIFY(m_NullableValue);
            return m_NullableValue;
        }

        constexpr T const* operator->() const
        {
            VERIFY(m_NullableValue);
            return m_NullableValue;
        }

        constexpr T& Value() const
        {
            return *m_NullableValue;
        }

        constexpr T ValueOr(T const& default_value) const
        {
            return m_NullableValue ? *m_NullableValue : default_value;
        }

        constexpr bool HasValue() const { return m_NullableValue != nullptr; }
        constexpr bool IsEmpty() const { return m_NullableValue == nullptr; }

        constexpr bool operator !() const { return m_NullableValue == nullptr; }

        constexpr explicit operator bool() const
        {
            return HasValue();
        }

        constexpr friend bool operator==(Maybe const& a, Maybe const& b)
        {
            if (a.IsEmpty() && b.IsEmpty()) {
                return true;
            }
            if (a.HasValue() && b.HasValue()) {
                return *a == *b;
            }
            return false;
        }

        constexpr friend bool operator!=(Maybe const& a, Maybe const& b)
        {
            return !(a == b);
        }

    private:
        T* m_NullableValue;
        alignas(alignof(T)) char m_Storage[sizeof(T)];
    };
}

#if defined(FSN_CORE_USE_GLOBALLY)
using Fussion::Maybe;
#endif
