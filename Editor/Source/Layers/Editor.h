﻿#pragma once
#include "EditorCamera.h"
#include "EditorStyle.h"
#include "EditorWindows/AssetWindows/AssetWindow.h"
#include "EditorWindows/ConsoleWindow.h"
#include "EditorWindows/ContentBrowser.h"
#include "EditorWindows/InspectorWindow.h"
#include "EditorWindows/SceneTreeWindow.h"
#include "EditorWindows/ScriptsInspector.h"
#include "EditorWindows/ViewportWindow.h"
#include "Project/Project.h"
#include "SceneRenderer.h"
#include "Undo.h"

#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Core/Delegate.h>
#include <Fussion/Core/Layer.h>
#include <Fussion/Core/Types.h>
#include <Fussion/OS/FileWatcher.h>
#include <Fussion/Scene/Scene.h>

class RendererReport;
class AssetRegistryViewer;

class Editor final : public Fsn::Layer {
public:
    enum class PlayState {
        Editing,
        Playing,
        Paused,
        Detached,
    };

    UndoRedo Undo;
    Fussion::Delegate<> OnBeginPlay;
    Fussion::Delegate<> OnStopPlay;
    Fussion::Delegate<> OnResumePlay;
    Fussion::Delegate<> OnPaused;

    Fussion::DebugDrawContext DebugDrawContext {};

    static AssetPicker GenericAssetPicker;

    Editor();
    virtual ~Editor() override;

    virtual void OnStart() override;
    virtual void OnEnable() override;
    virtual void OnDisable() override;

    virtual void OnUpdate(f32) override;
    virtual void OnEvent(Fsn::Event&) override;

    virtual void OnDraw(Fussion::GPU::CommandEncoder& encoder) override;
    virtual void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    void Save() const;

    auto GetSceneRenderer() -> SceneRenderer& { return m_SceneRenderer; }

    void OpenAsset(Fussion::AssetHandle handle);

    template<std::derived_from<AssetWindow> T, typename... Args>
    void CreateAssetWindow(Fussion::AssetHandle handle, Args&&... args)
    {
        if (m_AssetWindows.contains(handle))
            return;
        m_AssetWindows[handle] = MakePtr<T>(handle, std::forward<Args>(args)...);
    }

    void SetPlayState(PlayState new_state);
    auto GetPlayState() const -> PlayState { return m_PlayState; }

    auto GetLogEntries() -> std::vector<Fsn::LogEntry>
    {
        auto entries = m_LogEntries;
        m_LogEntries.clear();
        return entries;
    }

    static void ChangeScene(Fsn::AssetRef<Fsn::Scene> scene);

    static void OnViewportResized(Vector2 const& new_size);

    static auto Self() -> Editor& { return *s_EditorInstance; }

    static auto GetCamera() -> EditorCamera& { return s_EditorInstance->m_Camera; }
    static auto GetProject() -> Project& { return s_EditorInstance->m_Project; }

    static auto ActiveScene() -> Ref<Fsn::Scene>&
    {
        if (s_EditorInstance->m_PlayState == PlayState::Playing) {
            return s_EditorInstance->m_PlayScene;
        }
        return s_EditorInstance->m_ActiveScene;
    }

    // Editor Windows
    static auto Viewport() -> ViewportWindow& { return *s_EditorInstance->m_ViewportWindow.get(); }
    static auto Inspector() -> InspectorWindow& { return *s_EditorInstance->m_InspectorWindow.get(); }
    static auto SceneTree() -> SceneTreeWindow& { return *s_EditorInstance->m_SceneWindow.get(); }
    static auto Console() -> ConsoleWindow& { return *s_EditorInstance->m_ConsoleWindow.get(); }
    static auto ContentBrowser() -> ContentBrowserWindow& { return *s_EditorInstance->m_ContentBrowser.get(); }

private:
    static Editor* s_EditorInstance;
    std::vector<Fsn::LogEntry> m_LogEntries {};

    Ref<Fsn::Scene> m_ActiveScene;
    Ref<Fsn::Scene> m_PlayScene;

    std::filesystem::path m_ActiveScenePath;
    SceneRenderer m_SceneRenderer;

    Project m_Project;
    EditorCamera m_Camera;

    Ptr<Fussion::FileWatcher> m_Watcher;

    Ptr<ViewportWindow> m_ViewportWindow;
    Ptr<InspectorWindow> m_InspectorWindow;
    Ptr<SceneTreeWindow> m_SceneWindow;
    Ptr<ConsoleWindow> m_ConsoleWindow;
    Ptr<ContentBrowserWindow> m_ContentBrowser;
    Ptr<RendererReport> m_RendererReport;

    Ptr<AssetRegistryViewer> m_AssetRegistryViewer;
    Ptr<ScriptsInspector> m_ScriptsInspector;

    std::unordered_map<Fussion::AssetHandle, Ptr<AssetWindow>> m_AssetWindows {};

    PlayState m_PlayState { PlayState::Editing };
    bool m_Detached { false };
    bool m_ShadowMapDisplayOpened { false };
};
