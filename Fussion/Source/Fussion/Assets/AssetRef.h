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

    [[nodiscard]]
    bool IsValid() const { return m_Handle != 0; }

    [[nodiscard]]
    bool IsLoaded() const;

    [[nodiscard]]
    AssetHandle Handle() const { return m_Handle; }

    bool IsVirtual() const;

    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual AssetType GetType() const = 0;

    /// Blocks the current thread until the asset has completed loading.
    /// This is useful in situations during one-shot actions where you get
    /// an asset and want it before continuing with your code.
    void WaitUntilLoaded() const;

protected:
    [[nodiscard]] Asset* GetRaw(AssetType type) const;

    AssetHandle m_Handle{ 0 };
    bool m_Loaded{false};
};

template<class T, typename M = Detail::AssetRefMarker>
class AssetRef final : public AssetRefBase {
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

    virtual AssetType GetType() const override { return T::GetStaticType(); }
};

}

namespace Fsn = Fussion;
