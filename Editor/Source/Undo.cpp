#include "EditorPCH.h"

#include "Undo.h"

void UndoRedo::undo()
{
    if (!m_undo_items.empty()) {
        auto item = m_undo_items.back();
        m_undo_items.pop_back();
        if (item.ptr == nullptr) {
            return;
        }

        // @Note This might use new data for the redo. It might be
        // preferable to use to old data in the undo.
        auto redo_item = item;
        std::memcpy(redo_item.data, redo_item.ptr, redo_item.size);
        m_redo_items.push_back(redo_item);

        std::memcpy(item.ptr, item.data, item.size);
    }
}

void UndoRedo::redo()
{
    if (!m_redo_items.empty()) {
        auto item = m_redo_items.back();
        m_redo_items.pop_back();
        if (item.ptr == nullptr) {
            return;
        }

        // @Note This might use new data for the redo. It might be
        // preferable to use to old data in the undo.
        auto redo_item = item;
        std::memcpy(redo_item.data, redo_item.ptr, redo_item.size);
        m_undo_items.push_back(redo_item);

        std::memcpy(item.ptr, item.data, item.size);
    }
}

void UndoRedo::commit_tag(std::string const& tag)
{
    if (m_single_items.contains(tag)) {
        auto& item = m_single_items.at(tag);
        if (std::memcmp(item.ptr, item.data, item.size) != 0) {
            m_undo_items.push_back(item);
        }
        m_single_items.erase(tag);
    }
}

void UndoRedo::commit()
{
    for (auto const& item : m_temp_items) {
        if (std::memcmp(item.ptr, item.data, item.size) != 0) {
            m_undo_items.push_back(item);
        }
    }
    m_temp_items.clear();
}
