#pragma once
#include "Fussion/Core/Uuid.h"
#include "Fussion/ReflRegistrar.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Serialization/ISerializable.h"

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
        friend ReflRegistrar;

    public:
        virtual ~Asset() override = default;

        [[nodiscard]]
        virtual AssetType GetType() const = 0;

        [[nodiscard]]
        AssetHandle GetHandle() const { return m_Handle; }

        void SetHandle(AssetHandle handle)
        {
            m_Handle = handle;
        }

        template<typename T>
        Ref<T> As()
        {
            return std::dynamic_pointer_cast<T>(shared_from_this());
        }

        [[nodiscard]]
        bool IsValid() const
        {
            return m_Handle != 0;
        }

    protected:
        AssetHandle m_Handle{ 0 };

        friend class AssetManagerBase;
    };
}
