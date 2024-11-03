#pragma once
#include <Fussion/Core/Concepts.h>

namespace Fussion {
    template<typename T>
    class Span {
    public:
        constexpr Span(T* ptr, size_t length) noexcept
            : m_Ptr(ptr)
            , m_Length(length)
        { }

        template<SpanCompatible<T> Rng>
        constexpr explicit(false) Span(Rng&& range) noexcept
            : m_Ptr(range.data())
            , m_Length(range.size())
        { }

        template<size_t Size>
        constexpr explicit(false) Span(T (&arr)[Size]) noexcept
            : m_Ptr(arr)
            , m_Length(Size)
        { }

        constexpr Span(Span const& other) noexcept
            : m_Ptr(other.m_Ptr)
            , m_Length(other.m_Length)
        { }

        constexpr Span(Span&& other) noexcept
            : m_Ptr(other.m_Ptr)
            , m_Length(other.m_Length)
        { }

        constexpr Span& operator=(Span const& other)
        {
            if (this == &other)
                return *this;
            m_Ptr = other.m_Ptr;
            m_Length = other.m_Length;
            return *this;
        }

        constexpr Span& operator=(Span&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_Ptr = other.m_Ptr;
            m_Length = other.m_Length;
            return *this;
        }

        T& operator[](size_t index) const
        {
            VERIFY(index < m_Length);
            return m_Ptr[index];
        }

        size_t Length() const
        {
            return m_Length;
        }

        size_t SizeInBytes() const
        {
            return m_Length * sizeof(std::remove_cvref_t<T>);
        }

        T* DataPtr() const
        {
            return m_Ptr;
        }

        template<typename Y>
        bool operator==(Span<Y> const& other) const
        {
            if (other.m_Length != m_Length)
                return false;
            for (size_t i = 0; i < m_Length; ++i) {
                if (m_Ptr[i] != other[i])
                    return false;
            }
            return false;
        }

        Span SubSlice(size_t start, size_t end)
        {
            VERIFY(start <= end, "start: {}, end: {}", start, end);
            VERIFY(end < m_Length);
            return Span(m_Ptr + start, end - start);
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
            return Iterator(m_Ptr);
        }

        Iterator end()
        {
            return Iterator(m_Ptr + m_Length);
        }

    private:
        T* m_Ptr {};
        size_t m_Length {};
    };

    template<typename T>
    using ReadOnlySpan = Span<T const>;
}

#if FSN_CORE_USE_GLOBALLY
using Fussion::ReadOnlySpan;
using Fussion::Span;
#endif
