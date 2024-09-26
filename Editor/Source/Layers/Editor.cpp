#include "EditorPCH.h"
#include "Editor.h"
#include "EditorUI.h"
#include "EditorWindows/AssetWindows/MaterialWindow.h"
#include "EditorWindows/AssetWindows/Texture2DWindow.h"
#include "EditorWindows/AssetRegistryViewer.h"
#include "EditorWindows/RendererReport.h"

#include <Fussion/Input/Input.h>
#include <Fussion/Core/Application.h>
#include <Fussion/Scene/Components/BaseComponents.h>
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Events/KeyboardEvents.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scripting/ScriptingEngine.h>
#include <Fussion/Scene/Components/Camera.h>
#include <Fussion/Serialization/JsonSerializer.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <tracy/Tracy.hpp>
#include <ranges>

Editor* Editor::s_editor_instance = nullptr;
AssetPicker Editor::generic_asset_picker{};

using namespace Fussion;

Editor::Editor()
{
    VERIFY(s_editor_instance == nullptr, "EditorLayer already exists!")
    s_editor_instance = this;
}

Editor::~Editor() = default;

void Editor::on_start()
{
    Application::inst()->window().set_title(fmt::format("Fussion - {}", Project::name()));
    Application::inst()->window().maximize();

    ZoneScoped;
    m_viewport_window = make_ptr<ViewportWindow>(this);
    m_inspector_window = make_ptr<InspectorWindow>(this);
    m_console_window = make_ptr<ConsoleWindow>(this);
    m_scene_window = make_ptr<SceneTreeWindow>(this);
    m_scripts_inspector = make_ptr<ScriptsInspector>(this);
    m_content_browser = make_ptr<ContentBrowser>(this);
    m_asset_registry_viewer = make_ptr<AssetRegistryViewer>(this);
    m_renderer_report = make_ptr<RendererReport>(this);

    ScriptingEngine::inst().compile_game_assembly(Project::scripts_folder());
    FileSystem::write_entire_file(Project::scripts_folder() / "as.predefined", ScriptingEngine::inst().dump_current_types().str());

    m_watcher = FileWatcher::create(Project::scripts_folder());
    m_watcher->register_listener([](std::filesystem::path const& path, FileWatcher::EventType type) {
        using namespace std::chrono_literals;
        (void)path;
        (void)type;

        // Wait a bit for the file lock to be released.
        std::this_thread::sleep_for(100ms);
        ScriptingEngine::inst().compile_game_assembly(Project::scripts_folder());
    });
    m_watcher->start();

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");

    m_camera.resize(Application::inst()->window().size());
    m_camera.position = Vector3(0, 3, 5);
    m_scene_renderer.init();

    on_viewport_resized(Vector2(300, 300));

    m_viewport_window->on_start();
    m_inspector_window->on_start();
    m_console_window->on_start();
    m_scene_window->on_start();
    m_content_browser->on_start();

    m_asset_registry_viewer->on_start();
    m_asset_registry_viewer->hide();

    m_scripts_inspector->on_start();
    m_scripts_inspector->hide();

    on_begin_play += [this] {
        LOG_DEBUG("On Begin Play");
        auto meta = Project::asset_manager()->get_metadata(m_active_scene->handle());

        JsonDeserializer ds(*FileSystem::read_entire_file(Project::assets_folder() / meta.path));
        m_play_scene = make_ref<Scene>();
        m_play_scene->deserialize(ds);

        if (m_play_scene)
            m_play_scene->on_start();
    };

    on_stop_play += [this] {
        LOG_DEBUG("On Stop Play");
        if (m_play_scene) {
            m_play_scene = nullptr;
        }
    };

    on_paused += [] {
        LOG_DEBUG("On Paused");
    };

    on_resume_play += [] {
        LOG_DEBUG("On Resume Play");
    };
}

void Editor::on_enable() {}

void Editor::on_disable() {}

void Editor::save() const
{
    for (auto& asd : m_asset_windows) {
        asd.second->on_save();
    }

    Project::save();

    if (m_state == PlayState::Editing && m_active_scene != nullptr) {
        LOG_DEBUGF("Saving scene {} to {}", m_active_scene->name(), m_active_scene_path);
        JsonSerializer js;
        js.initialize();

        m_active_scene->serialize(js);

        auto path = Project::assets_folder() / m_active_scene_path;
        FileSystem::write_entire_file(path, js.to_string());

        m_active_scene->set_dirty(false);
    }
}

void Editor::open_asset(AssetHandle handle)
{
    if (m_asset_windows.contains(handle))
        return;

    auto assman = Project::asset_manager();
    auto meta = assman->get_metadata(handle);
    if (!meta.is_valid())
        return;

    switch (meta.type) {
    case AssetType::PbrMaterial:
        m_asset_windows[handle] = make_ptr<MaterialWindow>(handle);
        break;
    case AssetType::Texture2D:
        m_asset_windows[handle] = make_ptr<Texture2DWindow>(handle);
        break;
    default:
        break;
    }
}

void Editor::on_update(f32 delta)
{
    ZoneScoped;

    generic_asset_picker.update();

    switch (m_state) {
    case PlayState::Editing: {
        m_camera.set_focus(m_viewport_window->is_focused());
        m_camera.on_update(delta);
        if (m_active_scene) {
            m_active_scene->tick();
            m_active_scene->on_debug_draw(debug_draw_context);
        }
    }
    break;
    case PlayState::Playing: {
        if (m_detached) {
            m_camera.set_focus(m_viewport_window->is_focused());
            m_camera.on_update(delta);
        }
        if (m_play_scene) {
            m_play_scene->on_update(delta);
            m_play_scene->on_debug_draw(debug_draw_context);
        }
    }
    break;
    case PlayState::Paused: {}
    case PlayState::Detached: {}
    break;
    }

    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New..")) {
                if (ImGui::MenuItem("Create Scene")) {
                    change_scene(Project::asset_manager()->create_asset<Scene>("TestScene.fsn"));
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (!m_active_scene)
                ImGui::BeginDisabled();

            if (ImGui::MenuItem("Save..", "Ctrl+S")) {
                save();
            }

            if (!m_active_scene)
                ImGui::EndDisabled();

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) {}

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::MenuItem("Scripts Inspector")) {
                m_scripts_inspector->show();
            }
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Shadow Map")) {
                    m_shadow_map_display_opened = !m_shadow_map_display_opened;
                }

                if (ImGui::MenuItem("Asset Registry")) {
                    m_asset_registry_viewer->toggle();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Extra")) {
            ImGui::Checkbox("Demo Window", &show_demo_window);
            if (ImGui::MenuItem("Save Layout")) {
                ImGui::SaveIniSettingsToDisk("Assets/EditorLayout.ini");
            }

            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

    auto flags =
        ImGuiDockNodeFlags_NoTabBar |
        ImGuiDockNodeFlags_HiddenTabBar |
        ImGuiDockNodeFlags_NoDockingOverMe |
        ImGuiDockNodeFlags_NoResize |
        ImGuiDockNodeFlags_NoDockingOverOther;

    ImGuiWindowClass klass;
    klass.DockNodeFlagsOverrideSet = flags;

    ImGui::SetNextWindowClass(&klass);

    auto state = m_state;
    if (state == PlayState::Playing) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Color::Yellow);
    }
    EUI::window("##toolbar", [this] {
        constexpr auto y = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2{ 0, y });
        defer(ImGui::PopStyleVar());
        auto height = ImGui::GetWindowHeight() - y * 2;
        auto pos = ImGui::GetWindowContentRegionMax().x * 0.5f - 3.f * height * 0.5f;
        ImGui::SetCursorPosX(pos);
        ImGui::SetCursorPosY(y + 1);

        auto list = ImGui::GetWindowDrawList();
        list->ChannelsSplit(2);
        list->ChannelsSetCurrent(1);

        auto& style = EditorStyle::get_style();
        EUI::image_button(style.editor_icons[EditorIcon::Play], [this] {
            set_play_state(PlayState::Playing);
        }, { .size = Vector2{ height, height }, .disabled = m_active_scene == nullptr || m_state == PlayState::Playing });

        auto min = ImGui::GetItemRectMin();

        ImGui::SetItemTooltip("Begin Play mode. This will simulate the game with the in-game camera.");

        ImGui::SameLine();

        EUI::image_button(style.editor_icons[EditorIcon::Stop], [this] {
            set_play_state(PlayState::Editing);
        }, { .size = Vector2{ height, height }, .disabled = m_state != PlayState::Playing });

        ImGui::SameLine();

        EUI::image_button(style.editor_icons[EditorIcon::Pause], [this] {
            set_play_state(PlayState::Paused);
        }, { .size = Vector2{ height, height }, .disabled = m_state != PlayState::Playing });

        ImGui::SameLine();

        EUI::image_button(style.editor_icons[EditorIcon::Dots], [] {
            ImGui::OpenPopup("Toolbar::Options");
        }, { .size = Vector2{ height, height }, .disabled = m_state != PlayState::Playing });

        EUI::popup("Toolbar::Options", [&] {
            if (ImGui::MenuItem("Detach")) {
                m_detached = !m_detached;
            }
        });

        ImGui::SetItemTooltip("Pause");

        auto max = ImGui::GetItemRectMax();

        list->ChannelsSetCurrent(0);

        Color color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        color = color.lighten(0.2f);

        list->AddRectFilled(min, max, color.to_abgr(), 3);

        list->ChannelsMerge();
    }, { .flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize, .use_child = false });
    if (state == PlayState::Playing) {
        ImGui::PopStyleColor();
    }

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // TODO: Is there a better place to put this?
    // if (m_ShadowMapDisplayOpened) {
    //     EUI::Window("Shadow Map", [&] {
    //         EUI::Property("Cascade Index", &m_SceneRenderer.RenderDebugOptions.CascadeIndex, EUI::PropTypeRange{ .Min = 0.0f, .Max = ShadowCascades - 1 });
    //         ImGui::Image(IMGUI_IMAGE(m_SceneRenderer.GetShadowDebugFrameBuffer()->GetColorAttachment(0)), ImGui::GetContentRegionAvail());
    //     }, { .Opened = &m_ShadowMapDisplayOpened });
    // }

    m_viewport_window->on_draw();
    m_inspector_window->on_draw();
    m_console_window->on_draw();
    m_scene_window->on_draw();
    m_scripts_inspector->on_draw();
    m_content_browser->on_draw();
    m_renderer_report->on_draw();

    if (m_asset_registry_viewer->is_visible())
        m_asset_registry_viewer->on_draw();

    for (auto const& asset_window : m_asset_windows | std::views::values) {
        asset_window->draw(delta);
    }

    std::erase_if(m_asset_windows, [](auto const& pair) {
        return !pair.second->is_open();
    });

    undo.commit();

    Project::asset_manager()->check_for_loaded_assets();
}

void Editor::on_event(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (e.key == Keys::Z && e.mods.test(KeyMod::Control)) {
            undo.undo();
        }

        if (e.key == Keys::Y && e.mods.test(KeyMod::Control)) {
            undo.redo();
        }

        if (e.key == Keys::S && e.mods.test(KeyMod::Control)) {
            save();
        }

        // TODO: This is really fucky. Figure out a way to properly define what has input focus and what not.
        if (m_scene_window->is_focused() && !m_viewport_window->is_focused() && e.key == Keys::D && e.mods.test(KeyMod::Control)) {
            for (auto const& [entity, nothing] : m_scene_window->selection()) {
                (void)nothing;
                auto new_handle = m_active_scene->clone_entity(entity);
                if (new_handle != EntityHandle::Invalid) {
                    auto* new_entity = m_active_scene->get_entity(new_handle);
                    new_entity->name += " (Clone)";
                }
            }
        }
        return false;
    });

    dispatcher.dispatch<WindowCloseRequest>([this](WindowCloseRequest const&) {
        if (m_active_scene != nullptr && m_active_scene->is_dirty()) {
            Dialogs::MessageBox data{};
            data.message = "The current scene has unsaved modifications. Are you sure you want to quit?";
            data.action = Dialogs::MessageAction::OkCancel;
            switch (Dialogs::show_message_box(data)) {
            case Dialogs::MessageButton::Ok:
            case Dialogs::MessageButton::Yes:
                Application::inst()->quit();
                break;
            case Dialogs::MessageButton::No:
                break;
            case Dialogs::MessageButton::Cancel:
                break;
            }
        } else {
            Application::inst()->quit();
        }
        return true;
    });

    m_camera.handle_event(event);

    m_viewport_window->on_event(event);
    m_inspector_window->on_event(event);
    m_console_window->on_event(event);
    m_scene_window->on_event(event);
    m_scripts_inspector->on_event(event);
    m_content_browser->on_event(event);
}

void Editor::on_draw(GPU::CommandEncoder& encoder)
{
    auto RenderEditorView = [&](Ref<Scene> const& scene) {
        m_scene_renderer.render(encoder, {
            .camera = RenderCamera{
                .perspective = m_camera.perspective(),
                .view = m_camera.view(),
                .position = m_camera.position,
                .near = m_camera.near,
                .far = m_camera.far,
                .direction = m_camera.direction(),
            },
            .scene = scene.get(),
        }, false);
    };
    auto RenderGameView = [&](Camera const& camera) {
        auto entity = camera.owner();
        m_scene_renderer.render(encoder, {
            .camera = RenderCamera{
                .perspective = camera.get_perspective(),
                .view = inverse(entity->world_matrix()),
                .position = entity->transform.position,
                .near = camera.near,
                .far = camera.far,
                .direction = entity->transform.forward(),
            },
            .scene = m_play_scene.get(),
        }, true);
    };
    switch (m_state) {
    case PlayState::Editing: {
        RenderEditorView(m_active_scene);
    }
    break;
    case PlayState::Detached:
    case PlayState::Playing:
    case PlayState::Paused: {
        auto camera = m_play_scene->find_first_component<Camera>();

        if (camera == nullptr || m_detached) {
            RenderEditorView(m_play_scene);
        } else {
            RenderGameView(*camera.get());
        }
    }
    break;
    }

}

void Editor::set_play_state(PlayState new_state)
{
    ZoneScoped;
    switch (m_state) {
    case PlayState::Editing:
        switch (new_state) {
        case PlayState::Playing:
            on_begin_play.fire();
            break;
        // noop
        case PlayState::Editing:
        case PlayState::Paused:
        case PlayState::Detached:
            return;
        }
        break;
    case PlayState::Playing:
        switch (new_state) {
        case PlayState::Editing:
            on_stop_play.fire();
            break;
        case PlayState::Playing:
            // noop
            return;
        case PlayState::Paused:
            on_paused.fire();
            break;
        case PlayState::Detached:
            break;
        }
        break;
    case PlayState::Paused:
        switch (new_state) {
        case PlayState::Editing:
            on_stop_play.fire();
            break;
        case PlayState::Playing:
            on_resume_play.fire();
            break;
        case PlayState::Detached:
            break;
        case PlayState::Paused:
            // noop
            return;
        }
        break;
    case PlayState::Detached:
        switch (new_state) {
        case PlayState::Editing:
            break;
        case PlayState::Playing:
            break;
        case PlayState::Paused:
            break;
        case PlayState::Detached:
            // noop
            return;
        }
        break;
    }
    m_state = new_state;
}

void Editor::on_log_received(LogLevel level, std::string_view message, std::source_location const& loc)
{
    m_log_entries.push_back(LogEntry{
        level,
        std::string(message),
        loc,
    });
}

void Editor::change_scene(AssetRef<Scene> scene)
{
    auto LoadScene = [&scene] {
        s_editor_instance->m_scene_window->clear_selection();

        auto meta = Project::asset_manager()->get_metadata(scene.handle());

        if (auto scene_json = FileSystem::read_entire_file(Project::assets_folder() / meta.path)) {
            JsonDeserializer ds(*scene_json);

            auto scene_asset = make_ref<Scene>();
            scene_asset->deserialize(ds);
            scene_asset->set_handle(scene.handle());

            s_editor_instance->m_active_scene_path = meta.path;
            s_editor_instance->m_active_scene = scene_asset;

            Application::inst()->window().set_title(fmt::format("Fussion - {} - {}", Project::name(), meta.name));
        }
    };
    if (s_editor_instance->m_active_scene != nullptr && s_editor_instance->m_active_scene->is_dirty()) {
        Dialogs::MessageBox data{};
        data.message = "The current scene has unsaved modifications. Are you sure you want to discard them? Selecting 'No' will save the current scene and load the new one.";
        data.action = Dialogs::MessageAction::YesNoCancel;
        switch (Dialogs::show_message_box(data)) {
        case Dialogs::MessageButton::No: { s_editor_instance->save(); }
        case Dialogs::MessageButton::Yes: {
            LoadScene();
        }
        break;
        case Dialogs::MessageButton::Cancel:
            break;
        default:
            UNIMPLEMENTED;
        }
    } else {
        LoadScene();
    }
}

void Editor::on_viewport_resized(Vector2 const& new_size)
{
    ZoneScoped;

    s_editor_instance->m_camera.resize(new_size);
    s_editor_instance->m_scene_renderer.resize(new_size);
}
