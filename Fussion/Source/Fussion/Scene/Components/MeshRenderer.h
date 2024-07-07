#pragma once
#include "Assets/PbrMaterial.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Mesh.h"
#include "Renderer/FrameAllocator.h"

namespace Fussion {
class MeshRenderer : public Component {
    struct PushConstantData {
        Mat4 Model;
    } m_Data;
public:
    COMPONENT_DEFAULT(MeshRenderer)

    void OnCreate() override;
    void OnUpdate(f32 delta) override;
    void OnDraw(RenderContext& ctx) override;

    AssetRef<Mesh> Mesh;
    AssetRef<PbrMaterial> Material;

private:
    Ref<Shader> m_PbrShader;
    Ref<ResourceLayout> m_ObjectLayout;
    FrameAllocator m_FrameAllocator;
};
}
