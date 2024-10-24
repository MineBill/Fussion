#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Mem.h>
#include <Fussion/Math/Math.h>

#include <type_traits>

namespace Fussion {
    template<typename T>
    class DynamicArray {
    public:
        DynamicArray() = default;
        explicit DynamicArray(Mem::Allocator const& allocator): m_allocator(allocator) {}

        ~DynamicArray()
        {
            if (m_leaked) {
                return;
            }
            Mem::Free(m_buffer, m_allocator);
        }

        void init(Mem::Allocator const& allocator)
        {
            m_allocator = allocator;
        }

        template<std::same_as<T> V>
        void append(V const& value)
        {
            ensure_capacity(m_length + 1);

            m_buffer[m_length++] = value;
        }

        /// Ensure the array can contain at least size items.
        /// If necessary, this will reallocate.
        void ensure_capacity(usz size)
        {
            if (size < m_capacity) {
                return;
            }

            if (m_capacity == 0) {
                m_capacity = 10;
            } else {
                m_capacity = Math::Max(m_capacity + m_capacity / 2, size);
            }

            auto new_buffer = Mem::Alloc<T>(m_capacity, m_allocator);
            Mem::Copy(new_buffer, m_buffer);

            Mem::Free(m_buffer, m_allocator);
            m_buffer = new_buffer;
        }

        /// Remove item at index and shift the remaining items.
        void remove_at(usz index)
        {
            if (index >= m_length)
                return;
            if (index == m_length - 1) {
                m_length--;
                return;
            }
            m_buffer[index].~T();
            Mem::Copy(m_buffer.ptr + index, m_buffer.ptr + index + 1, m_length * sizeof(T));
            m_length--;
        }

        T& operator[](usz index)
        {
            VERIFY(index < m_length);
            return m_buffer[index];
        }

        /// Returns the inner slice of items and prevent automatic deallocation
        /// in the destructor.
        [[nodiscard]]
        Slice<T> leak()
        {
            m_leaked = true;
            m_buffer.length = m_length;
            return m_buffer;
        }

        /// Returns the current length of the array.
        [[nodiscard]]
        usz len() const { return m_length; }

        /// Returns the current capacity of the array.
        [[nodiscard]]
        usz capacity() const { return m_capacity; }

        struct Iterator {
            explicit Iterator(T* ptr): m_ptr(ptr) {}

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
            return Iterator(m_buffer.ptr);
        }

        Iterator end()
        {
            return Iterator(m_buffer.ptr + m_leaked);
        }

    private:
        Mem::Allocator m_allocator{};
        Slice<T> m_buffer{};
        usz m_length{ 0 };
        usz m_capacity{ 0 };
        bool m_leaked{ false };
    };
}
