﻿#pragma once
#include "Fussion/Core/Application.h"
#include "Fussion/Core/Maybe.h"

#include <argparse.hpp>

namespace fs = std::filesystem;

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

class EditorApplication final : public Fussion::Application {
public:
    EditorApplication();

    virtual void OnStart() override;
    virtual void OnUpdate(f32 delta) override;
    virtual void OnEvent(Fussion::Event&) override;
    virtual void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    static auto CreateProject(Maybe<fs::path> path, std::string_view name) -> fs::path;

    static EditorApplication* Self() { return s_EditorInstance; }

    static void CreateEditor(Maybe<fs::path> path);
    static void CreateEditorFromProjectCreator(fs::path path);

private:
    static EditorApplication* s_EditorInstance;

    EditorCLI m_Args;
};
