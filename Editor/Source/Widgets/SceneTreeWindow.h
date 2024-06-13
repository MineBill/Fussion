#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow
{
public:
    EDITOR_WINDOW(SceneTreeWindow)

    void OnDraw() override;

    std::unordered_map<Fussion::UUID, Fussion::Entity*> const& GetSelection() const { return m_Selection; }
private:
    std::unordered_map<Fussion::UUID, Fussion::Entity*> m_Selection{};
};
