#pragma once
#include "AssetManagerBase.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/ReflRegistrar.h"

namespace Fussion {
    namespace Detail {
        struct AssetRefMarker {};
    }

    class AssetRefBase : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
        friend ReflRegistrar;

    public:
        AssetRefBase() = default;
        virtual ~AssetRefBase() = default;

        explicit AssetRefBase(AssetHandle handle)
            : m_handle(handle) {}

        operator bool() const { return is_valid(); }

        [[nodiscard]]
        bool is_valid() const { return m_handle != 0; }

        [[nodiscard]]
        bool is_loaded() const;

        [[nodiscard]]
        AssetHandle handle() const { return m_handle; }

        [[nodiscard]]
        bool is_virtual() const;

        void set_handle(AssetHandle handle) { m_handle = handle; }

        virtual AssetType type() const = 0;

        /// Blocks the current thread until the asset has completed loading.
        /// This is useful in situations during one-shot actions where you get
        /// an asset and want it before continuing with your code.
        void wait_until_loaded() const;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    protected:
        [[nodiscard]] Asset* get_raw(AssetType type) const;

        AssetHandle m_handle{ 0 };
        bool m_loaded{ false };
    };

    template<class T, typename M = Detail::AssetRefMarker>
    class AssetRef final : public AssetRefBase {
        META_HPP_ENABLE_POLY_INFO(AssetRefBase)
    public:
        AssetRef() : AssetRefBase() {}

        explicit AssetRef(AssetHandle handle): AssetRefBase(handle) {}

        T* get()
        {
            if (!is_valid())
                return nullptr;
            return CAST(T*, get_raw(T::static_type()));
        }

        T* get() const
        {
            if (!is_valid())
                return nullptr;
            return CAST(T*, get_raw(T::static_type()));
        }

        virtual AssetType type() const override { return T::static_type(); }
    };
}

namespace Fsn = Fussion;
