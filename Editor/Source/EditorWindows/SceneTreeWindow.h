#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class SceneTreeWindow final : public EditorWindow {
public:
    using SelectionList = std::unordered_map<Fussion::Uuid, std::monostate>;

    EDITOR_WINDOW(SceneTreeWindow)

    virtual void on_draw() override;

    [[nodiscard]]
    auto selection() const -> SelectionList const& { return m_selection; }

    void clear_selection() { m_selection.clear(); }
    void select_entity(Fussion::Uuid entity, bool clear = true);

private:
    void draw_entity_hierarchy(Fsn::Uuid handle);

    SelectionList m_selection{};
};
