#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

class UndoRedo {
    struct Item {
        void* ptr{};
        size_t size{};
        char data[512]{};
        std::string tag{};
    };

    std::vector<Item> m_undo_items;
    std::vector<Item> m_redo_items;
    std::vector<Item> m_temp_items;

    std::unordered_map<std::string, Item> m_single_items;

public:
    template<typename T>
    void push(T* data, std::string const& tag = "", size_t size = sizeof(T))
    {
        for (auto const& item : m_temp_items) {
            if (item.ptr == data) {
                return;
            }
        }

        Item item = {
            .ptr = data,
            .size = size,
            .tag = tag,
        };
        std::memcpy(item.data, data, size);
        m_temp_items.push_back(item);
    }

    template<typename T>
    void push_single(T* data, std::string const& tag, size_t size = sizeof(T))
    {
        for (auto const& item : m_temp_items) {
            if (item.ptr == data) {
                return;
            }
        }

        Item item = {
            .ptr = data,
            .size = size,
            .tag = tag,
        };
        std::memcpy(item.data, data, size);
        m_single_items[tag] = std::move(item);
    }

    void undo();
    void redo();

    void commit_tag(std::string const& tag);
    void commit();
};
