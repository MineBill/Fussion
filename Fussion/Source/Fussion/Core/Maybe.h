#pragma once
#include "Core.h"

#include <type_traits>

namespace Fussion {

    struct None {};

    template<typename T>
    class Maybe {

    public:
        explicit constexpr Maybe(): m_nullable_value(nullptr) {}
        constexpr Maybe(None): Maybe() {}

        constexpr ~Maybe() { reset(); }

        constexpr Maybe(T const& _value):
            m_nullable_value(new(m_storage) T(_value)) {}

        constexpr Maybe(T&& _value):
            m_nullable_value(new(m_storage) T(std::move(_value))) {}

        constexpr Maybe& operator=(T const& _value)
        {
            reset();
            m_nullable_value = new(m_storage) T(_value);
            return *this;
        }

        constexpr Maybe& operator=(T&& _value)
        {
            reset();
            m_nullable_value = new(m_storage) T(std::move(_value));
            return *this;
        }

        constexpr Maybe(Maybe const& _other):
            m_nullable_value(_other ? new(m_storage) T(*_other) : nullptr) {}

        constexpr Maybe(Maybe&& _other) noexcept:
            m_nullable_value(_other
                ? new(m_storage) T(std::move(*_other))
                : nullptr) {}

        constexpr Maybe& operator=(Maybe const& _other)
        {
            if (&_other != this) {
                reset();
                if (_other) { m_nullable_value = new(m_storage) T(*_other); }
            }
            return *this;
        }

        constexpr Maybe& operator=(Maybe&& _other) noexcept
        {
            if (&_other != this) {
                reset();
                if (_other) {
                    m_nullable_value = new(m_storage) T(std::move(*_other));
                }
            }
            return *this;
        }

        constexpr void reset()
        {
            if (m_nullable_value) { m_nullable_value->~T(); }
            m_nullable_value = nullptr;
        }

        constexpr T& operator*()
        {
            VERIFY(m_nullable_value, "Tried to access empty Maybe");
            return *m_nullable_value;
        }

        constexpr T const& operator*() const
        {
            VERIFY(m_nullable_value, "Tried to access empty Maybe");
            return *m_nullable_value;
        }

        constexpr T* operator->()
        {
            VERIFY(m_nullable_value, "Tried to access empty Maybe");
            return m_nullable_value;
        }

        constexpr T const* operator->() const
        {
            VERIFY(m_nullable_value, "Tried to access empty Maybe");
            return m_nullable_value;
        }

        constexpr T& unwrap() const
        {
            VERIFY(m_nullable_value, "Tried to access empty Maybe");
            return *m_nullable_value;
        }

        constexpr T value_or(T const& default_value) const
        {
            return m_nullable_value ? *m_nullable_value : default_value;
        }

        constexpr bool has_value() const { return m_nullable_value != nullptr; }
        constexpr bool is_empty() const { return m_nullable_value == nullptr; }

        constexpr bool operator !() const { return m_nullable_value == nullptr; }

        constexpr explicit operator bool() const
        {
            return has_value();
        }

        constexpr friend bool operator==(Maybe const& a, Maybe const& b)
        {
            if (a.is_empty() && b.is_empty()) {
                return true;
            }
            if (a.has_value() && b.has_value()) {
                return *a == *b;
            }
            return false;
        }

        constexpr friend bool operator!=(Maybe const& a, Maybe const& b)
        {
            return !(a == b);
        }

    private:
        T* m_nullable_value;
        alignas(alignof(T)) char m_storage[sizeof(T)];
    };
}

#if defined(FSN_CORE_USE_GLOBALLY)
using Fussion::Maybe;
#endif
