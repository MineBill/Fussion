#pragma once
#include "AssetManagerBase.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/ReflRegistrar.h"

namespace Fussion {
namespace Detail {
struct AssetRefMarker {};
}

class AssetRefBase {
    META_HPP_ENABLE_POLY_INFO()
    friend ReflRegistrar;

public:
    AssetRefBase() = default;
    virtual ~AssetRefBase() = default;

    explicit AssetRefBase(AssetHandle handle)
        : m_Handle(handle) {}

    operator bool() const { return IsValid(); }

    [[nodiscard]] bool IsValid() const { return m_Handle != 0; }
    [[nodiscard]] AssetHandle Handle() const { return m_Handle; }

    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual AssetType GetType() = 0;

protected:
    [[nodiscard]] Asset* GetRaw(AssetType type) const;

    AssetHandle m_Handle{ 0 };
};

template<class T, typename M = Detail::AssetRefMarker>
class AssetRef : public AssetRefBase {
    META_HPP_ENABLE_POLY_INFO(AssetRefBase)
public:
    AssetRef() : AssetRefBase() {}

    explicit AssetRef(AssetHandle handle): AssetRefBase(handle) {}

    T* Get()
    {
        if (!IsValid())
            return nullptr;
        return CAST(T*, GetRaw(T::GetStaticType()));
    }

    T* Get() const
    {
        if (!IsValid())
            return nullptr;
        return CAST(T*, GetRaw(T::GetStaticType()));
    }

    AssetType GetType() override { return T::GetStaticType(); }
};

}

namespace Fsn = Fussion;
