#pragma once
#include "Fussion/Core/Application.h"
#include "Fussion/Core/Maybe.h"
#include "Layers/ImGuiLayer.h"

#include <argparse.hpp>

namespace fs = std::filesystem;

struct EditorCLI final : argparse::Args {
    EditorCLI& operator=(EditorCLI const& other)
    {
        project_path = other.project_path;
        create_project = other.create_project;
        return *this;
    }

    std::optional<std::string>& project_path = kwarg("p,project", "Path to the project");
    bool& create_project = flag("c,create", "Create a new project at <ProjectPath>").set_default(false);
};

static_assert(std::is_move_assignable_v<EditorCLI>);

class Editor;

class EditorApplication : public Fussion::Application {
public:
    EditorApplication();

    virtual void on_start() override;
    virtual void on_update(f32 delta) override;
    virtual void on_event(Fussion::Event&) override;
    virtual void on_log_received(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    static auto create_project(Maybe<fs::path> path, std::string_view name) -> fs::path;

    static EditorApplication* inst() { return s_editor_instance; }

    static void create_editor(Maybe<fs::path> path);
    static void create_editor_from_project_creator(fs::path path);

private:
    static EditorApplication* s_editor_instance;

    ImGuiLayer* m_im_gui_layer{};
    EditorCLI m_args;
    // Ptr<Editor> m_Editor{};
};
