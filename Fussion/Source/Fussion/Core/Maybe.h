#pragma once
#include "Core.h"

#include <type_traits>

namespace Fussion {
    template<typename T>
    class Maybe {
    public:
        Maybe(): m_NullableValue(nullptr) {}
        ~Maybe() { Reset(); }

        Maybe(T const& _value):
            m_NullableValue(new(m_Storage) T(_value)) {}

        Maybe(T&& _value):
            m_NullableValue(new(m_Storage) T(std::move(_value))) {}

        Maybe& operator=(T const& _value)
        {
            Reset();
            m_NullableValue = new(m_Storage) T(_value);
            return *this;
        }

        Maybe& operator=(T&& _value)
        {
            Reset();
            m_NullableValue = new(m_Storage) T(std::move(_value));
            return *this;
        }

        Maybe(Maybe const& _other):
            m_NullableValue(_other ? new(m_Storage) T(*_other) : nullptr) {}

        Maybe(Maybe&& _other) noexcept:
            m_NullableValue(_other
                ? new(m_Storage) T(std::move(*_other))
                : nullptr) {}

        Maybe& operator=(Maybe const& _other)
        {
            if (&_other != this) {
                Reset();
                if (_other) { m_NullableValue = new(m_Storage) T(*_other); }
            }
            return *this;
        }

        Maybe& operator=(Maybe&& _other) noexcept
        {
            if (&_other != this) {
                Reset();
                if (_other) {
                    m_NullableValue = new(m_Storage) T(std::move(*_other));
                }
            }
            return *this;
        }

        void Reset()
        {
            if (m_NullableValue) { m_NullableValue->~T(); }
            m_NullableValue = nullptr;
        }

        T& operator*()
        {
            VERIFY(m_NullableValue);
            return *m_NullableValue;
        }

        T const& operator*() const
        {
            VERIFY(m_NullableValue);
            return *m_NullableValue;
        }

        T* operator->()
        {
            VERIFY(m_NullableValue);
            return m_NullableValue;
        }

        T const* operator->() const
        {
            VERIFY(m_NullableValue);
            return m_NullableValue;
        }

        T Value() const
        {
            return *m_NullableValue;
        }

        T ValueOr(T const& default_value) const
        {
            return m_NullableValue ? *m_NullableValue : default_value;
        }

        bool HasValue() const { return m_NullableValue != nullptr; }
        bool IsEmpty() const { return m_NullableValue == nullptr; }

        bool operator !() const { return m_NullableValue == nullptr; }

        explicit operator bool() const
        {
            return HasValue();
        }

        friend bool operator==(Maybe const& a, Maybe const& b)
        {
            if (a.IsEmpty() && b.IsEmpty()) {
                return true;
            }
            if (a.HasValue() && b.HasValue()) {
                return *a == *b;
            }
            return false;
        }

        friend bool operator!=(Maybe const& a, Maybe const& b)
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
