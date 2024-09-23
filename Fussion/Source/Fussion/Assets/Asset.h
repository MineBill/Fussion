#pragma once
#include "Fussion/Core/Uuid.h"
#include "Fussion/Reflection/ReflectionRegistry.h"
#include "Fussion/Serialization/ISerializable.h"
#include "Fussion/meta.hpp/meta_all.hpp"

namespace Fussion {
    enum class [[nodiscard]] AssetType {
        Invalid,

        Image,
        Script,
        Model,
        PbrMaterial,
        Scene,
        Shader,
        Texture,
        Texture2D,
        HDRTexture,
    };

    using AssetHandle = Uuid;

    class AssetMetadata : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
    public:
        virtual ~AssetMetadata() override = default;
    };

    class [[nodiscard]] Asset : public std::enable_shared_from_this<Asset>, public ISerializable {
        friend ReflectionRegistry;

    public:
        virtual ~Asset() override = default;

        [[nodiscard]]
        virtual AssetType type() const = 0;

        [[nodiscard]]
        AssetHandle handle() const { return m_handle; }

        void set_handle(AssetHandle handle)
        {
            m_handle = handle;
        }

        template<typename T>
        Ref<T> as()
        {
            return std::dynamic_pointer_cast<T>(shared_from_this());
        }

        [[nodiscard]]
        bool is_valid() const
        {
            return m_handle != 0;
        }

    protected:
        AssetHandle m_handle{ 0 };

        friend class AssetManagerBase;
    };
}
