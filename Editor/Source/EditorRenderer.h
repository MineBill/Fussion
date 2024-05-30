#pragma once
#include "Engin5/Core/Types.h"
#include "Engin5/Renderer/RenderPass.h"

class EditorRenderer
{
public:
    void Init();

    Ref<Engin5::RenderPass> const& GetMainRenderPass() const { return m_MainRenderPass; }
    Ref<Engin5::RenderPass> const& GetUIRenderPass() const { return m_UIRenderPass; }
private:
    Ref<Engin5::RenderPass> m_MainRenderPass{}, m_UIRenderPass{};
};
