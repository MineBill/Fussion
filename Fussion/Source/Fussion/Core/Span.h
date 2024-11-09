#pragma once
#include <Fussion/Core/Concepts.h>

namespace Fussion {
    template<typename T>
    class Span {
    public:
        constexpr Span() = default;
        constexpr Span(T* ptr, size_t length) noexcept
            : m_ptr(ptr)
            , m_length(length)
        { }

        template<SpanCompatible<T> Rng>
        constexpr explicit(false) Span(Rng&& range) noexcept
            : m_ptr(range.data())
            , m_length(range.size())
        { }

        template<size_t Size>
        constexpr explicit(false) Span(T (&arr)[Size]) noexcept
            : m_ptr(arr)
            , m_length(Size)
        { }

        constexpr Span(Span const& other) noexcept
            : m_ptr(other.m_ptr)
            , m_length(other.m_length)
        { }

        constexpr Span(Span&& other) noexcept
            : m_ptr(other.m_ptr)
            , m_length(other.m_length)
        { }

        constexpr Span& operator=(Span const& other)
        {
            if (this == &other)
                return *this;
            m_ptr = other.m_ptr;
            m_length = other.m_length;
            return *this;
        }

        constexpr Span& operator=(Span&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_ptr = other.m_ptr;
            m_length = other.m_length;
            return *this;
        }

        T& operator[](size_t index) const
        {
            VERIFY(index < m_length);
            return m_ptr[index];
        }

        size_t size() const
        {
            return m_length;
        }

        size_t size_in_bytes() const
        {
            return m_length * sizeof(std::remove_cvref_t<T>);
        }

        T* data() const
        {
            return m_ptr;
        }

        template<typename Y>
        bool operator==(Span<Y> const& other) const
        {
            if (other.m_length != m_length)
                return false;
            for (size_t i = 0; i < m_length; ++i) {
                if (m_ptr[i] != other[i])
                    return false;
            }
            return true;
        }

        Span slice(size_t start, size_t end)
        {
            VERIFY(start <= end, "start: {}, end: {}", start, end);
            VERIFY(end <= m_length);
            return Span(m_ptr + start, end - start);
        }

        void reset()
        {
            m_ptr = nullptr;
            m_length = 0;
        }

        struct Iterator {
            explicit Iterator(T* ptr)
                : m_Ptr(ptr)
            { }

            Iterator& operator++()
            {
                (void)m_Ptr++;
                return *this;
            }

            T operator++(int)
            {
                Iterator copy = *this;
                ++*this;
                return copy;
            }

            Iterator& operator--()
            {
                (void)m_Ptr--;
                return *this;
            }

            T operator--(int)
            {
                Iterator copy = *this;
                --*this;
                return copy;
            }

            T& operator*()
            {
                return *m_Ptr;
            }

            T* operator->()
            {
                return m_Ptr;
            }

            bool operator==(Iterator const& other) const
            {
                return m_Ptr == other.m_Ptr;
            }

        private:
            T* m_Ptr;
        };

        Iterator begin()
        {
            return Iterator(m_ptr);
        }

        Iterator end()
        {
            return Iterator(m_ptr + m_length);
        }

    private:
        T* m_ptr {};
        size_t m_length {};
    };

    template<typename T>
    using ReadOnlySpan = Span<T const>;
}

#if FSN_CORE_USE_GLOBALLY
using Fussion::ReadOnlySpan;
using Fussion::Span;
#endif
