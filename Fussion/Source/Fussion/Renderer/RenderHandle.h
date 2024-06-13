#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

namespace Fussion
{
    class RenderHandle: public std::enable_shared_from_this<RenderHandle>
    {
    public:
        RenderHandle();
        virtual ~RenderHandle() = default;

        // virtual void Destroy() = 0;


        require_results bool Equals(const RenderHandle& other) const;
        require_results bool Equals(const Ref<RenderHandle>& other) const;

        /**
         * Helper function to downcast to backend-specific types.
         * @tparam T
         * @return
         */
        template<class T>
        Ref<T> As()
        {
            EASSERT(this != nullptr, "*this is a nullptr");
            return std::dynamic_pointer_cast<T>(shared_from_this());
        }

        template<class T>
        T GetRenderHandle()
        {
            return transmute(T, GetRawHandle());
        }

        virtual void* GetRawHandle() = 0;

    protected:
        u64 m_Id{};
    };
}
