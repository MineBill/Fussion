#pragma once
#include <Fussion/Core/Types.h>
#include <Fussion/Core/Concepts.h>

namespace Fussion {
    class RefCounted {
    public:
        void AddRef();

        void Release();

        u32 GetRefCount() const;

    protected:
        virtual ~RefCounted() = default;

    private:
        u32 m_RefCount{ 0 };
    };

    template<std::derived_from<RefCounted> T>
    class RefPtr {
    public:
        RefPtr() : m_Ptr(nullptr) {}

        RefPtr(T* obj) : m_Ptr(obj)
        {
            if (m_Ptr) {
                m_Ptr->AddRef();
            }
        }

        RefPtr(RefPtr const& other) : m_Ptr(other.m_Ptr)
        {
            if (m_Ptr) {
                m_Ptr->AddRef();
            }
        }

        RefPtr(RefPtr&& other) noexcept : m_Ptr(other.m_Ptr)
        {
            other.m_Ptr = nullptr;
        }

        ~RefPtr()
        {
            if (m_Ptr) {
                m_Ptr->Release();
            }
        }

        RefPtr& operator=(RefPtr const& other)
        {
            if (this != &other) {
                if (m_Ptr) {
                    m_Ptr->Release();
                }
                m_Ptr = other.m_Ptr;
                if (m_Ptr) {
                    m_Ptr->AddRef();
                }
            }
            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            if (this != &other) {
                if (m_Ptr) {
                    m_Ptr->Release();
                }
                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

        T* operator->() const { return m_Ptr; }
        T& operator*() const { return *m_Ptr; }

        T* Leak() const { return m_Ptr; }

    private:
        T* m_Ptr;
    };

    template<std::derived_from<RefCounted> T, typename... Args>
    RefPtr<T> MakeRefPtr(Args&&... args)
    {
        T* obj = new T(std::forward<Args>(args)...);
        return RefPtr<T>(obj);
    }

}

#if defined(FSN_CORE_USE_GLOBALLY)
using Fussion::RefPtr;
using Fussion::RefCounted;
using Fussion::MakeRefPtr;
#endif
