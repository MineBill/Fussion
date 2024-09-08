#pragma once
#include <Fussion/Core/Types.h>
#include <Fussion/Core/Concepts.h>

namespace Fussion {
    class RefCounted {
    public:
        void add_ref();

        void release();

        u32 ref_count() const;

    protected:
        virtual ~RefCounted() = default;

    private:
        u32 m_ref_count{ 0 };
    };

    template<std::derived_from<RefCounted> T>
    class RefPtr {
    public:
        RefPtr() : m_ptr(nullptr) {}

        RefPtr(T* obj) : m_ptr(obj)
        {
            if (m_ptr) {
                m_ptr->add_ref();
            }
        }

        RefPtr(RefPtr const& other) : m_ptr(other.m_ptr)
        {
            if (m_ptr) {
                m_ptr->add_ref();
            }
        }

        RefPtr(RefPtr&& other) noexcept : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~RefPtr()
        {
            if (m_ptr) {
                m_ptr->release();
            }
        }

        RefPtr& operator=(RefPtr const& other)
        {
            if (this != &other) {
                if (m_ptr) {
                    m_ptr->release();
                }
                m_ptr = other.m_ptr;
                if (m_ptr) {
                    m_ptr->add_ref();
                }
            }
            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            if (this != &other) {
                if (m_ptr) {
                    m_ptr->release();
                }
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        T* operator->() const { return m_ptr; }
        T& operator*() const { return *m_ptr; }

        T* leak() const { return m_ptr; }

    private:
        T* m_ptr;
    };

    template<std::derived_from<RefCounted> T, typename... Args>
    RefPtr<T> make_ref_ptr(Args&&... args)
    {
        T* obj = new T(std::forward<Args>(args)...);
        return RefPtr<T>(obj);
    }

}

#if defined(FSN_CORE_USE_GLOBALLY)
using Fussion::RefPtr;
using Fussion::RefCounted;
using Fussion::make_ref_ptr;
#endif
