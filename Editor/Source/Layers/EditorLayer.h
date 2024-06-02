#pragma once
#include "Widgets/ViewportWindow.h"

#include "Engin5/Core/Layer.h"
#include "Engin5/Core/Types.h"
#include "Engin5/Renderer/CommandBuffer.h"
#include "Widgets/ConsoleWindow.h"
#include "Widgets/InspectorWindow.h"
#include "Widgets/SceneWindow.h"

class EditorLayer: public Engin5::Layer
{
public:
    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;

    void OnUpdate(f32) override;
    void OnEvent(Engin5::Event&) override;

    void OnDraw(Ref<Engin5::CommandBuffer> cmd);
    ViewportWindow& GetViewport() { return *m_ViewportWindow.get(); }

private:
    Ptr<ViewportWindow> m_ViewportWindow;
    Ptr<InspectorWindow> m_InspectorWindow;
    Ptr<SceneWindow> m_SceneWindow;
    Ptr<ConsoleWindow> m_ConsoleWindow;


};
