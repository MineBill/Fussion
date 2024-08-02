#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow {
public:
    using SelectionList = std::unordered_map<Fussion::Uuid, std::monostate>;

    EDITOR_WINDOW(SceneTreeWindow)

    virtual void OnDraw() override;

    [[nodiscard]] SelectionList const& GetSelection() const { return m_Selection; }
    void ClearSelection() { m_Selection.clear(); }

private:
    void DrawEntityHierarchy(Fsn::Uuid handle);
    void SelectEntity(Fussion::Entity* entity, bool clear = true);

    SelectionList m_Selection{};
};
