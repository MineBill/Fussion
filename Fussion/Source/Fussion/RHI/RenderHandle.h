#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

namespace Fussion::RHI {
    class RenderHandle : public std::enable_shared_from_this<RenderHandle> {
    public:
        RenderHandle();
        virtual ~RenderHandle() = default;

        [[nodiscard]]
        bool Equals(RenderHandle const& other) const;

        [[nodiscard]]
        bool Equals(Ref<RenderHandle> const& other) const;

        /**
         * Helper function to downcast to backend-specific types.
         * @tparam T
         * @return
         */
        template<class T>
        Ref<T> As()
        {
            return std::dynamic_pointer_cast<T>(shared_from_this());
        }

        template<class T>
        Ref<T> As() const
        {
            return std::dynamic_pointer_cast<T>(shared_from_this());
        }

        template<class T>
        T GetRenderHandle()
        {
            return TRANSMUTE(T, GetRawHandle());
        }

        virtual void* GetRawHandle() = 0;

    protected:
        u64 m_Id{};
    };
}
