#pragma once
#include <Fussion/Core/Core.h>

namespace Fussion {
    template<typename T>
    struct Slice {
        T* ptr;
        usz length;

        Slice() = default;

        Slice(T* ptr, usz length)
            : ptr(ptr)
            , length(length)
        { }

        Slice(Slice const& other)
            : ptr(other.ptr)
            , length(other.length)
        { }

        Slice(Slice&& other) noexcept
            : ptr(other.ptr)
            , length(other.length)
        { }

        Slice& operator=(Slice const& other)
        {
            if (this == &other)
                return *this;
            ptr = other.ptr;
            length = other.length;
            return *this;
        }

        Slice& operator=(Slice&& other) noexcept
        {
            if (this == &other)
                return *this;
            ptr = other.ptr;
            length = other.length;
            return *this;
        }

        T& operator[](usz index) const
        {
            VERIFY(index < length);
            return ptr[index];
        }

        usz len() const
        {
            return length;
        }

        template<typename Y>
        bool operator==(Slice<Y> const& other) const
        {
            if (other.length != length)
                return false;
            for (usz i = 0; i < length; ++i) {
                if (ptr[i] != other[i])
                    return false;
            }
            return false;
        }

        Slice SubSlice(usz start, usz end)
        {
            VERIFY(start <= end, "start: {}, end: {}", start, end);
            return Slice(ptr + start, end - start);
        }

        struct Iterator {
            Iterator(T* ptr)
                : m_ptr(ptr)
            { }

            Iterator& operator++()
            {
                (void)m_ptr++;
                return *this;
            }

            T operator++(int)
            {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            Iterator& operator--()
            {
                (void)m_ptr--;
                return *this;
            }

            T operator--(int)
            {
                Iterator copy = *this;
                --(*this);
                return copy;
            }

            T& operator*()
            {
                return *m_ptr;
            }

            T* operator->()
            {
                return m_ptr;
            }

            bool operator==(Iterator const& other) const
            {
                return m_ptr == other.m_ptr;
            }

        private:
            T* m_ptr;
        };

        Iterator begin()
        {
            return Iterator(ptr);
        }

        Iterator end()
        {
            return Iterator(ptr + length);
        }
    };
}

#if FSN_CORE_USE_GLOBALLY
using Fussion::Slice;
#endif
