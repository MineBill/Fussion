#pragma once
#include "AssetManagerBase.h"

namespace Engin5
{
    template<typename T>
    class AssetRef
    {
        static_assert(std::derived_from<T, Asset>, "T must be of type Asset!");
    public:
        AssetRef() = default;
        explicit AssetRef(AssetManagerBase* asset_manager) : m_AssetManager(asset_manager) {}

        T* Get()
        {
            return cast(T*, m_AssetManager->GetAsset(m_Handle, T::GetStaticType()));
        }

        require_results bool IsValid() const { return m_AssetManager != nullptr; }
        require_results AssetHandle Handle() const { return m_Handle; }

    private:
        AssetManagerBase* m_AssetManager{};
        AssetHandle m_Handle{};
    };
}
