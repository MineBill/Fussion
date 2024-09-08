#pragma once
#include "EditorCamera.h"
#include "EditorStyle.h"
#include "SceneRenderer.h"
#include "Undo.h"
#include "EditorWindows/ConsoleWindow.h"
#include "EditorWindows/ContentBrowser.h"
#include "EditorWindows/InspectorWindow.h"
#include "EditorWindows/SceneTreeWindow.h"
#include "EditorWindows/ScriptsInspector.h"
#include "EditorWindows/ViewportWindow.h"
#include "EditorWindows/AssetWindows/AssetWindow.h"
#include "Project/Project.h"

#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Core/Delegate.h>
#include <Fussion/Core/Layer.h>
#include <Fussion/Core/Types.h>
#include <Fussion/OS/FileWatcher.h>
#include <Fussion/RHI/CommandBuffer.h>
#include <Fussion/Scene/Scene.h>

class AssetRegistryViewer;

class Editor final : public Fsn::Layer {
public:
    enum class PlayState {
        Editing,
        Playing,
        Paused,
        Detached,
    };

    UndoRedo undo;
    Fussion::Delegate<> on_begin_play;
    Fussion::Delegate<> on_stop_play;
    Fussion::Delegate<> on_resume_play;
    Fussion::Delegate<> on_paused;

    Fussion::DebugDrawContext debug_draw_context{};

    static AssetPicker generic_asset_picker;

    Editor();
    virtual ~Editor() override;

    virtual void on_start() override;
    virtual void on_enable() override;
    virtual void on_disable() override;

    virtual void on_update(f32) override;
    virtual void on_event(Fsn::Event&) override;

    virtual void on_draw(Fussion::GPU::CommandEncoder& encoder) override;
    virtual void on_log_received(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    void save() const;

    auto scene_renderer() -> SceneRenderer& { return m_scene_renderer; }

    void open_asset(Fussion::AssetHandle handle);

    template<std::derived_from<AssetWindow> T, typename... Args>
    void create_asset_window(Fussion::AssetHandle handle, Args&&... args)
    {
        if (m_asset_windows.contains(handle))
            return;
        m_asset_windows[handle] = MakePtr<T>(handle, std::forward<Args>(args)...);
    }

    void set_play_state(PlayState new_state);
    auto play_state() const -> PlayState { return m_state; }

    auto log_entries() -> std::vector<Fsn::LogEntry>
    {
        auto entries = m_log_entries;
        m_log_entries.clear();
        return entries;
    }

    static void change_scene(Fsn::AssetRef<Fsn::Scene> scene);

    static void on_viewport_resized(Vector2 const& new_size);

    static auto inst() -> Editor& { return *s_editor_instance; }

    static auto camera() -> EditorCamera& { return s_editor_instance->m_camera; }
    static auto project() -> Project& { return s_editor_instance->m_project; }

    static auto active_scene() -> Ref<Fsn::Scene>&
    {
        if (s_editor_instance->m_state == PlayState::Playing) {
            return s_editor_instance->m_play_scene;
        }
        return s_editor_instance->m_active_scene;
    }

    // Editor Windows
    static auto viewport() -> ViewportWindow& { return *s_editor_instance->m_viewport_window.get(); }
    static auto inspector() -> InspectorWindow& { return *s_editor_instance->m_inspector_window.get(); }
    static auto scene_tree() -> SceneTreeWindow& { return *s_editor_instance->m_scene_window.get(); }
    static auto console() -> ConsoleWindow& { return *s_editor_instance->m_console_window.get(); }
    static auto content_browser() -> ContentBrowser& { return *s_editor_instance->m_content_browser.get(); }

private:
    static Editor* s_editor_instance;
    std::vector<Fsn::LogEntry> m_log_entries{};

    Ref<Fsn::Scene> m_active_scene;
    Ref<Fsn::Scene> m_play_scene;

    std::filesystem::path m_active_scene_path;
    SceneRenderer m_scene_renderer;

    Project m_project;
    EditorCamera m_camera;

    Ptr<Fussion::FileWatcher> m_watcher;

    Ptr<ViewportWindow> m_viewport_window;
    Ptr<InspectorWindow> m_inspector_window;
    Ptr<SceneTreeWindow> m_scene_window;
    Ptr<ConsoleWindow> m_console_window;
    Ptr<ContentBrowser> m_content_browser;

    Ptr<AssetRegistryViewer> m_asset_registry_viewer;
    Ptr<ScriptsInspector> m_scripts_inspector;

    std::unordered_map<Fussion::AssetHandle, Ptr<AssetWindow>> m_asset_windows{};

    PlayState m_state{ PlayState::Editing };
    bool m_detached{ false };
    bool m_shadow_map_display_opened{ false };
};
