#pragma once
#include "EditorCamera.h"
#include "EditorStyle.h"
#include "Widgets/ViewportWindow.h"
#include "Widgets/ConsoleWindow.h"
#include "Widgets/InspectorWindow.h"
#include "Widgets/SceneTreeWindow.h"
#include "Project/Project.h"
#include "SceneRenderer.h"
#include "Widgets/ScriptsInspector.h"

#include "Fussion/Core/Layer.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Renderer/CommandBuffer.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Scene.h"
#include "Widgets/ContentBrowser.h"

class Editor: public Fsn::Layer
{
public:
    Editor();

    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;

    void OnUpdate(f32) override;
    void OnEvent(Fsn::Event&) override;

    void OnDraw(Ref<Fsn::CommandBuffer> cmd);
    void Quit();

    EditorStyle& GetStyle() { return m_Style; }
    SceneRenderer& GetSceneRenderer() { return m_SceneRenderer; }

    void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc);

    std::vector<Fsn::LogEntry> GetLogEntries()
    {
        auto entries = m_LogEntries;
        m_LogEntries.clear();
        return entries;
    }

    static void OnViewportResized(Vector2 new_size);

    static Editor& Get() { return *s_EditorInstance; }

    static EditorCamera&             GetCamera()      { return s_EditorInstance->m_Camera; }
    static Project&                  GetProject()     { return s_EditorInstance->m_Project; }
    static Fsn::AssetRef<Fsn::Scene> GetActiveScene() { return s_EditorInstance->m_ActiveScene; }

    // Editor Windows
    static ViewportWindow&  GetViewport()       { return *s_EditorInstance->m_ViewportWindow.get(); }
    static InspectorWindow& GetInspector()      { return *s_EditorInstance->m_InspectorWindow.get(); }
    static SceneTreeWindow& GetSceneTree()      { return *s_EditorInstance->m_SceneWindow.get(); }
    static ConsoleWindow&   GetConsole()        { return *s_EditorInstance->m_ConsoleWindow.get(); }
    static ContentBrowser&  GetContentBrowser() { return *s_EditorInstance->m_ContentBrowser.get(); }

private:
    static Editor* s_EditorInstance;
    std::vector<Fsn::LogEntry> m_LogEntries{};

    Fsn::AssetRef<Fsn::Scene> m_ActiveScene;
    SceneRenderer m_SceneRenderer;

    Project m_Project;
    EditorCamera m_Camera;

    EditorStyle m_Style{};
    Ptr<ViewportWindow> m_ViewportWindow;
    Ptr<InspectorWindow> m_InspectorWindow;
    Ptr<SceneTreeWindow> m_SceneWindow;
    Ptr<ConsoleWindow> m_ConsoleWindow;
    Ptr<ContentBrowser> m_ContentBrowser;

    Ptr<ScriptsInspector> m_ScriptsInspector;
};
