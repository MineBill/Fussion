#pragma once
#include "Fussion/Core/Application.h"
#include "Fussion/Scene/Scene.h"
#include "Layers/ImGuiLayer.h"
#include "Layers/Editor.h"

class Editor;
class EditorApplication: public Fussion::Application
{
public:
    void OnStart() override;
    void OnUpdate(f32 delta) override;
    void OnEvent(Fussion::Event&) override;
    void OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc) override;

    static EditorApplication* Instance() { return s_EditorInstance; }

private:
    static EditorApplication* s_EditorInstance;

    Ptr<ImGuiLayer> m_ImGuiLayer{};
    Ptr<Editor> m_Editor{};
};
