#pragma once
#include "AssetManagerBase.h"

namespace Fussion
{
    template<class T>
    class AssetRef
    {
        static_assert(std::derived_from<T, Asset>, "T must be of type Asset!");
    public:
        AssetRef() = default;
        explicit AssetRef(AssetHandle handle, AssetManagerBase* asset_manager)
            : m_Handle(handle), m_AssetManager(asset_manager) {}

        operator bool() const { return IsValid(); }

        T* Get()
        {
            return CAST(T*, m_AssetManager->GetAsset(m_Handle, T::GetStaticType()));
        }

        require_results bool IsValid() const { return m_AssetManager != nullptr; }
        require_results AssetHandle Handle() const { return m_Handle; }

    private:
        AssetHandle m_Handle{};
        AssetManagerBase* m_AssetManager{};
    };
}

namespace Fsn = Fussion;