#pragma once
#include "EditorWindow.h"
#include "Fussion/Assets/Texture2D.h"

class ConsoleWindow final : public EditorWindow {
public:
    EDITOR_WINDOW(ConsoleWindow)

    virtual void OnStart() override;
    virtual void OnDraw() override;

private:
    bool m_info_enable{ true }, m_warning_enabled{ true }, m_error_enabled{ true };
    bool m_auto_scroll{ true };
    std::vector<Fsn::LogEntry> m_log_entries;
};
