#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow {
public:
    using SelectionList = std::unordered_map<Fussion::UUID, Fussion::Entity*>;

    EDITOR_WINDOW(SceneTreeWindow)

    void OnDraw() override;

    [[nodiscard]] SelectionList const& GetSelection() const { return m_Selection; }

private:
    void DrawEntityHierarchy(Fsn::UUID handle);

    SelectionList m_Selection{};
};
