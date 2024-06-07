#pragma once
#include "Engin5/Core/Application.h"
#include "Engin5/Scene/Scene.h"
#include "Layers/ImGuiLayer.h"
#include "Layers/Editor.h"

class Editor;
class EditorApplication: public Engin5::Application
{
public:
    void OnStart() override;
    void OnUpdate(f32 delta) override;
    void OnEvent(Engin5::Event&) override;
    void OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc) override;

    static EditorApplication* Instance() { return s_EditorInstance; }

private:
    static EditorApplication* s_EditorInstance;

    Ptr<ImGuiLayer> m_ImGuiLayer{};
    Ptr<Editor> m_Editor{};
};
