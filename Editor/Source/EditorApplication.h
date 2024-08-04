#pragma once
#include "Fussion/Core/Application.h"
#include "Fussion/Core/Maybe.h"
#include "Fussion/Scene/Scene.h"
#include "Layers/ImGuiLayer.h"
#include "Layers/Editor.h"

class Editor;
class EditorApplication: public Fussion::Application
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(f32 delta) override;
    virtual void OnEvent(Fussion::Event&) override;
    virtual void OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc) override;

    static EditorApplication* Instance() { return s_EditorInstance; }

    static void CreateEditor(Maybe<std::filesystem::path> path);

private:
    static EditorApplication* s_EditorInstance;

    ImGuiLayer* m_ImGuiLayer{};
    // Ptr<Editor> m_Editor{};
};
