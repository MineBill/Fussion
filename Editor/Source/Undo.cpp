﻿#include "Undo.h"

void UndoRedo::Undo()
{
    if (!m_UndoItems.empty()) {
        auto item = m_UndoItems.back();
        m_UndoItems.pop_back();
        if (item.Ptr == nullptr) {
            return;
        }

        // @Note This might use new data for the redo. It might be
        // preferable to use to old data in the undo.
        auto redo_item = item;
        std::memcpy(redo_item.Data, redo_item.Ptr, redo_item.Size);
        m_RedoItems.push_back(redo_item);

        std::memcpy(item.Ptr, item.Data, item.Size);
    }
}

void UndoRedo::Redo()
{
    if (!m_RedoItems.empty()) {
        auto item = m_RedoItems.back();
        m_RedoItems.pop_back();
        if (item.Ptr == nullptr) {
            return;
        }

        // @Note This might use new data for the redo. It might be
        // preferable to use to old data in the undo.
        auto redo_item = item;
        std::memcpy(redo_item.Data, redo_item.Ptr, redo_item.Size);
        m_UndoItems.push_back(redo_item);

        std::memcpy(item.Ptr, item.Data, item.Size);
    }
}

void UndoRedo::CommitTag(std::string const& tag)
{
    if (m_SingleItems.contains(tag)) {
        auto& item = m_SingleItems.at(tag);
        if (std::memcmp(item.Ptr, item.Data, item.Size) != 0) {
            m_UndoItems.push_back(item);
        }
        m_SingleItems.erase(tag);
    }
}

void UndoRedo::Commit()
{
    for (auto const& item : m_TempItems) {
        if (std::memcmp(item.Ptr, item.Data, item.Size) != 0) {
            m_UndoItems.push_back(item);
        }
    }
    m_TempItems.clear();
}