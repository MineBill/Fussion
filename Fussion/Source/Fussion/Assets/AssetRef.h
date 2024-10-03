#pragma once
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Reflection/ReflectionRegistry.h"
#include "Fussion/meta.hpp/meta_all.hpp"

namespace Fussion {
    namespace Detail {
        struct AssetRefMarker { };
    }

    class AssetRefBase : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
        friend ReflectionRegistry;

    public:
        AssetRefBase() = default;
        virtual ~AssetRefBase() = default;

        explicit AssetRefBase(AssetHandle handle);

        operator bool() const { return IsValid(); }

        [[nodiscard]]
        bool IsValid() const
        {
            return m_IsValid && m_Handle != 0;
        }

        [[nodiscard]]
        bool IsLoaded() const;

        [[nodiscard]]
        AssetHandle GetHandle() const
        {
            return m_Handle;
        }

        [[nodiscard]]
        bool IsVirtual() const;

        void SetHandle(AssetHandle handle);

        virtual AssetType GetType() const = 0;

        /// Blocks the current thread until the asset has completed loading.
        /// This is useful in situations during one-shot actions where you get
        /// an asset and want it before continuing with your code.
        void WaitUntilLoaded() const;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    protected:
        [[nodiscard]] Asset* GetRaw(AssetType type) const;

        AssetHandle m_Handle { 0 };
        bool m_IsValid { true };
        bool m_Loaded { false };
    };

    template<class T, typename M = Detail::AssetRefMarker>
    class AssetRef final : public AssetRefBase {
        META_HPP_ENABLE_POLY_INFO(AssetRefBase)
    public:
        AssetRef()
            : AssetRefBase()
        {
        }

        explicit AssetRef(AssetHandle handle)
            : AssetRefBase(handle)
        {
        }

        T* Get()
        {
            if (!IsValid())
                return nullptr;
            return CAST(T*, GetRaw(T::StaticType()));
        }

        T* Get() const
        {
            if (!IsValid())
                return nullptr;
            return CAST(T*, GetRaw(T::StaticType()));
        }

        virtual AssetType GetType() const override { return T::StaticType(); }
    };
}

namespace Fsn = Fussion;
