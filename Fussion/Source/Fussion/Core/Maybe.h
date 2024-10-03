#pragma once
#include "Core.h"

#include <type_traits>

namespace Fussion {

    struct None { };

    template<typename T>
    class Maybe {

    public:
        explicit constexpr Maybe()
            : m_Value(nullptr)
        {
        }
        constexpr Maybe(None)
            : Maybe()
        {
        }

        constexpr ~Maybe() { Reset(); }

        constexpr Maybe(T const& _value)
            : m_Value(new(m_Storage) T(_value))
        {
        }

        constexpr Maybe(T&& _value)
            : m_Value(new(m_Storage) T(std::move(_value)))
        {
        }

        constexpr Maybe& operator=(T const& _value)
        {
            Reset();
            m_Value = new (m_Storage) T(_value);
            return *this;
        }

        constexpr Maybe& operator=(T&& _value)
        {
            Reset();
            m_Value = new (m_Storage) T(std::move(_value));
            return *this;
        }

        constexpr Maybe(Maybe const& _other)
            : m_Value(_other ? new(m_Storage) T(*_other) : nullptr)
        {
        }

        constexpr Maybe(Maybe&& _other) noexcept
            : m_Value(_other
                      ? new(m_Storage) T(std::move(*_other))
                      : nullptr)
        {
        }

        constexpr Maybe& operator=(Maybe const& _other)
        {
            if (&_other != this) {
                Reset();
                if (_other) {
                    m_Value = new (m_Storage) T(*_other);
                }
            }
            return *this;
        }

        constexpr Maybe& operator=(Maybe&& _other) noexcept
        {
            if (&_other != this) {
                Reset();
                if (_other) {
                    m_Value = new (m_Storage) T(std::move(*_other));
                }
            }
            return *this;
        }

        constexpr void Reset()
        {
            if (m_Value) {
                m_Value->~T();
            }
            m_Value = nullptr;
        }

        constexpr T& operator*()
        {
            VERIFY(m_Value, "Tried to access empty Maybe");
            return *m_Value;
        }

        constexpr T const& operator*() const
        {
            VERIFY(m_Value, "Tried to access empty Maybe");
            return *m_Value;
        }

        constexpr T* operator->()
        {
            VERIFY(m_Value, "Tried to access empty Maybe");
            return m_Value;
        }

        constexpr T const* operator->() const
        {
            VERIFY(m_Value, "Tried to access empty Maybe");
            return m_Value;
        }

        constexpr T& Unwrap() const
        {
            VERIFY(m_Value, "Tried to access empty Maybe");
            return *m_Value;
        }

        constexpr T ValueOr(T const& default_value) const
        {
            return m_Value ? *m_Value : default_value;
        }

        constexpr bool HasValue() const { return m_Value != nullptr; }
        constexpr bool IsEmpty() const { return m_Value == nullptr; }

        constexpr bool operator!() const { return m_Value == nullptr; }

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
        T* m_Value;
        alignas(alignof(T)) char m_Storage[sizeof(T)];
    };
}

#if defined(FSN_CORE_USE_GLOBALLY)
using Fussion::Maybe;
#endif
