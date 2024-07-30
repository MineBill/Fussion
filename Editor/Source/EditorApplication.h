#pragma once
#include "Fussion/Core/Application.h"
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

private:
    static EditorApplication* s_EditorInstance;

    Ptr<ImGuiLayer> m_ImGuiLayer{};
    Ptr<Editor> m_Editor{};
};
