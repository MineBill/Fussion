#pragma once
#include "Fussion/Core/Application.h"
#include "Fussion/Core/Maybe.h"
#include "Layers/ImGuiLayer.h"

#include <argparse.hpp>

struct EditorCLI final : argparse::Args {
    EditorCLI& operator=(EditorCLI const& other)
    {
        ProjectPath = other.ProjectPath;
        CreateProject = other.CreateProject;
        return *this;
    }

    std::optional<std::string>& ProjectPath = kwarg("p,project", "Path to the project");
    bool& CreateProject = flag("c,create", "Create a new project at <ProjectPath>").set_default(false);
};

static_assert(std::is_move_assignable_v<EditorCLI>);

class Editor;

class EditorApplication : public Fussion::Application {
public:
    EditorApplication();

    virtual void OnStart() override;
    virtual void OnUpdate(f32 delta) override;
    virtual void OnEvent(Fussion::Event&) override;
    virtual void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    static EditorApplication* Instance() { return s_EditorInstance; }

    static void CreateEditor(Maybe<std::filesystem::path> path);

private:
    static EditorApplication* s_EditorInstance;

    ImGuiLayer* m_ImGuiLayer{};
    EditorCLI m_Args;
    // Ptr<Editor> m_Editor{};
};
