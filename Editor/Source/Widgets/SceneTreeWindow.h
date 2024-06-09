#pragma once
#include "EditorWindow.h"
#include "Engin5/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow
{
public:
    EDITOR_WINDOW(SceneTreeWindow)

    void OnDraw() override;

    std::unordered_map<Engin5::UUID, Engin5::Entity*> const& GetSelection() const { return m_Selection; }
private:
    std::unordered_map<Engin5::UUID, Engin5::Entity*> m_Selection{};
};
