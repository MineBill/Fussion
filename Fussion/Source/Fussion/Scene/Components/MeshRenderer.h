#pragma once
#include "Assets/PbrMaterial.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"
#include "RHI/FrameAllocator.h"

namespace Fussion {
class MeshRenderer : public Component {
    struct PushConstantData {
        Mat4 Model;
    } m_Data;

    struct DepthPushConstantData {
        Mat4 Model;
        Mat4 LightSpace;
    } m_DepthPushData;
public:
    COMPONENT_DEFAULT(MeshRenderer)

    void OnCreate() override;
    void OnUpdate(f32 delta) override;
    void OnDraw(RHI::RenderContext& ctx) override;

    AssetRef<Mesh> Mesh;
    AssetRef<PbrMaterial> Material;

private:
    Ref<RHI::Shader> m_PbrShader;
    Ref<RHI::ResourceLayout> m_ObjectLayout;
    RHI::FrameAllocator m_FrameAllocator;
};
}
