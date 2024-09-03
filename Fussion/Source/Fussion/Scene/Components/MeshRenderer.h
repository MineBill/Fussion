#pragma once
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/RHI/FrameAllocator.h"

namespace Fussion {
    class MeshRenderer final : public Component {
        struct PushConstantData {
            Mat4 Model;
        } m_Data;

        struct DepthPushConstantData {
            Mat4 Model;
            Mat4 LightSpace;
        } m_DepthPushData;

        struct ObjectPickingConstantData {
            Mat4 Model;
            s32 LocalID;
        } m_ObjectPickingPushData;

    public:
        COMPONENT_DEFAULT(MeshRenderer)

        virtual void OnStart() override;
        virtual void OnUpdate(f32 delta) override;
        virtual void OnDraw(RenderContext& ctx) override;

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        AssetRef<Model> Model;

        std::vector<AssetRef<PbrMaterial>> Materials{};

        virtual auto Clone() -> Ref<Component> override;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
    };
}
