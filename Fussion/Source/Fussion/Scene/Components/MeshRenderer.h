#pragma once
#include "Assets/PbrMaterial.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"
#include "RHI/FrameAllocator.h"

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
    virtual void OnDraw(RHI::RenderContext& ctx) override;

    AssetRef<Mesh> Mesh;
    AssetRef<PbrMaterial> Material;

private:
    Ref<RHI::Shader> m_PbrShader;
    Ref<RHI::ResourceLayout> m_ObjectLayout;
    RHI::FrameAllocator m_FrameAllocator;
};
}
