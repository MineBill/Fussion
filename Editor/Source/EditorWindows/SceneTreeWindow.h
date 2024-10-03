#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow {
public:
    using SelectionList = std::unordered_map<Fussion::Uuid, std::monostate>;

    EDITOR_WINDOW(SceneTreeWindow)

    virtual void OnDraw() override;

    [[nodiscard]]
    auto GetSelection() const -> SelectionList const&
    {
        return m_Selection;
    }

    void ClearSelection() { m_Selection.clear(); }
    void SelectEntity(Fussion::Uuid entity, bool clear = true);

private:
    void DrawEntityHierarchy(Fsn::Uuid handle);

    SelectionList m_Selection {};
};
