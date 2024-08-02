#pragma once
#include "EditorWindow.h"
#include "Fussion/Assets/Texture2D.h"

class ConsoleWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(ConsoleWindow)

    void OnStart() override;
    void OnDraw() override;
private:
    bool m_InfoEnable{true}, m_WarningEnabled{true}, m_ErrorEnabled{true};
    bool m_AutoScroll{true};
    std::vector<Fsn::LogEntry> m_LogEntries;
};
