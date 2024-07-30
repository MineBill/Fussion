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
#include "Widgets/ContentBrowser.h"
#include "Undo.h"

#include "Fussion/Core/Layer.h"
#include "Fussion/Core/Types.h"
#include "Fussion/RHI/CommandBuffer.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Core/Delegate.h"
#include "Fussion/OS/FileWatcher.h"
#include "Fussion/Scene/Scene.h"
#include <memory_resource>

class AssetWindow;

class Editor : public Fsn::Layer {
public:
    enum class PlayState {
        Editing,
        Playing,
        Paused,
    };

    UndoRedo Undo;
    Fussion::Delegate<> OnBeginPlay;
    Fussion::Delegate<> OnStopPlay;
    Fussion::Delegate<> OnResumePlay;
    Fussion::Delegate<> OnPaused;

    Editor();

    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;
    void Save();

    void OnUpdate(f32) override;
    void OnEvent(Fsn::Event&) override;

    void OnDraw(Ref<Fsn::RHI::CommandBuffer> cmd);
    void Quit();

    EditorStyle& GetStyle() { return m_Style; }
    SceneRenderer& GetSceneRenderer() { return m_SceneRenderer; }

    template<std::derived_from<AssetWindow> T, typename... Args>
    void CreateAssetWindow(Fussion::AssetHandle handle, Args&&... args)
    {
        if (m_AssetWindows.contains(handle))
            return;
        m_AssetWindows[handle] = MakePtr<T>(handle, std::forward<Args>(args)...);
    }

    void SetPlayState(PlayState new_state);

    void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc);

    std::vector<Fsn::LogEntry> GetLogEntries()
    {
        auto entries = m_LogEntries;
        m_LogEntries.clear();
        return entries;
    }

    static void ChangeScene(Fsn::AssetRef<Fsn::Scene> scene);

    static void OnViewportResized(Vector2 new_size);

    static Editor& Get() { return *s_EditorInstance; }

    static EditorCamera& GetCamera() { return s_EditorInstance->m_Camera; }
    static Project& GetProject() { return s_EditorInstance->m_Project; }

    static Ref<Fsn::Scene>& GetActiveScene()
    {
        if (s_EditorInstance->m_State == PlayState::Playing) {
            return s_EditorInstance->m_PlayScene;
        }
        return s_EditorInstance->m_ActiveScene;
    }

    // Editor Windows
    static ViewportWindow& GetViewport() { return *s_EditorInstance->m_ViewportWindow.get(); }
    static InspectorWindow& GetInspector() { return *s_EditorInstance->m_InspectorWindow.get(); }
    static SceneTreeWindow& GetSceneTree() { return *s_EditorInstance->m_SceneWindow.get(); }
    static ConsoleWindow& GetConsole() { return *s_EditorInstance->m_ConsoleWindow.get(); }
    static ContentBrowser& GetContentBrowser() { return *s_EditorInstance->m_ContentBrowser.get(); }

    Fsn::AssetRef<Fsn::Texture2D> TextureRef;

    static std::pmr::monotonic_buffer_resource ArenaAllocator;

private:
    static Editor* s_EditorInstance;
    std::vector<Fsn::LogEntry> m_LogEntries{};

    Ref<Fsn::Scene> m_ActiveScene;
    Ref<Fsn::Scene> m_PlayScene;

    std::filesystem::path m_ActiveScenePath;
    SceneRenderer m_SceneRenderer;

    Project m_Project;
    EditorCamera m_Camera;

    Ptr<Fussion::FileWatcher> m_Watcher;

    EditorStyle m_Style{};
    Ptr<ViewportWindow> m_ViewportWindow;
    Ptr<InspectorWindow> m_InspectorWindow;
    Ptr<SceneTreeWindow> m_SceneWindow;
    Ptr<ConsoleWindow> m_ConsoleWindow;
    Ptr<ContentBrowser> m_ContentBrowser;

    Ptr<ScriptsInspector> m_ScriptsInspector;

    std::unordered_map<Fussion::AssetHandle, Ptr<AssetWindow>> m_AssetWindows{};

    PlayState m_State{ PlayState::Editing };
};
