#pragma once
#include <Fussion/Core/Core.h>

namespace Fussion {
    template<typename T>
    struct Slice {
        T* ptr;
        usz length;

        T& operator[](usz index) const
        {
            VERIFY(index < length);
            return ptr[index];
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

        struct Iterator {
            Iterator(T* ptr): m_ptr(ptr) {}

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
