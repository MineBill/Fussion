#pragma once
#include "EditorCamera.h"
#include "EditorStyle.h"
#include "Widgets/ViewportWindow.h"

#include "Engin5/Core/Layer.h"
#include "Engin5/Core/Types.h"
#include "Engin5/Renderer/CommandBuffer.h"
#include "Project/Project.h"
#include "Widgets/ConsoleWindow.h"
#include "Widgets/InspectorWindow.h"
#include "Widgets/SceneTreeWindow.h"
#include "SceneRenderer.h"
#include "Widgets/ScriptsInspector.h"

class Editor: public Engin5::Layer
{
public:
    Editor();

    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;

    void OnUpdate(f32) override;
    void OnEvent(Engin5::Event&) override;

    void OnDraw(Ref<Engin5::CommandBuffer> cmd);
    EditorStyle& GetStyle() { return m_Style; }
    SceneRenderer& GetSceneRenderer() { return m_SceneRenderer; }

    void OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc);

    std::vector<LogEntry> GetLogEntries()
    {
        auto entries = m_LogEntries;
        m_LogEntries.clear();
        return entries;
    }

    static void OnViewportResized(Vector2 new_size);

    static Editor& Get() { return *s_EditorInstance; }

    static EditorCamera&      GetCamera()      { return s_EditorInstance->m_Camera; }
    static Project&           GetProject()     { return s_EditorInstance->m_Project; }
    static Ref<Engin5::Scene> GetActiveScene() { return s_EditorInstance->m_ActiveScene; }

    // Editor Windows
    static ViewportWindow&  GetViewport()  { return *s_EditorInstance->m_ViewportWindow.get(); }
    static InspectorWindow& GetInspector() { return *s_EditorInstance->m_InspectorWindow.get(); }
    static SceneTreeWindow& GetSceneTree() { return *s_EditorInstance->m_SceneWindow.get(); }
    static ConsoleWindow&   GetConsole()   { return *s_EditorInstance->m_ConsoleWindow.get(); }

private:
    static Editor* s_EditorInstance;
    std::vector<LogEntry> m_LogEntries{};

    Ref<Engin5::Scene> m_ActiveScene;
    SceneRenderer m_SceneRenderer;

    Project m_Project;
    EditorCamera m_Camera;

    EditorStyle m_Style{};
    Ptr<ViewportWindow> m_ViewportWindow;
    Ptr<InspectorWindow> m_InspectorWindow;
    Ptr<SceneTreeWindow> m_SceneWindow;
    Ptr<ConsoleWindow> m_ConsoleWindow;

    Ptr<ScriptsInspector> m_ScriptsInspector;
};
