#pragma once
#include "EditorWindow.h"
#include "Engin5/Assets/Texture2D.h"

class ConsoleWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(ConsoleWindow)

    void OnStart() override;
    void OnDraw() override;
private:
    Ref<Engin5::Texture2D> m_ErrorIcon{}, m_InfoIcon{}, m_WarningIcon{};

    bool m_InfoEnable{true}, m_WarningEnabled{true}, m_ErrorEnabled{true};
    bool m_AutoScroll{true};
    std::vector<LogEntry> m_LogEntries;
};
