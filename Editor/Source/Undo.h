#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

class UndoRedo {
    struct Item {
        void* Ptr{};
        size_t Size{};
        char Data[512]{};
        std::string Tag{};
    };

    std::vector<Item> m_UndoItems;
    std::vector<Item> m_RedoItems;
    std::vector<Item> m_TempItems;

    std::unordered_map<std::string, Item> m_SingleItems;

public:
    template<typename T>
    void Push(T* data, std::string const& tag = "", size_t size = sizeof(T))
    {
        for (auto const& item : m_TempItems) {
            if (item.Ptr == data) {
                return;
            }
        }

        Item item = {
            .Ptr = data,
            .Size = size,
            .Tag = tag,
        };
        std::memcpy(item.Data, data, size);
        m_TempItems.push_back(item);
    }

    template<typename T>
    void PushSingle(T* data, std::string const& tag, size_t size = sizeof(T))
    {
        for (auto const& item : m_TempItems) {
            if (item.Ptr == data) {
                return;
            }
        }

        Item item = {
            .Ptr = data,
            .Size = size,
            .Tag = tag,
        };
        std::memcpy(item.Data, data, size);
        m_SingleItems[tag] = std::move(item);
    }

    void Undo();
    void Redo();

    void CommitTag(std::string const& tag);
    void Commit();
};
