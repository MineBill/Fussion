#pragma once
#include <Fussion/Core/Mem.h>
#include <string>
#include <unordered_map>
#include <vector>

class UndoRedo {
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
            .size = size,
            .Tag = tag,
        };
        Fussion::Mem::Copy(item.Data, data, size);
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
            .size = size,
            .Tag = tag,
        };
        Fussion::Mem::Copy(item.Data, data, size);
        m_SingleItems[tag] = std::move(item);
    }

    void Undo();
    void Redo();

    void CommitTag(std::string const& tag);
    void Commit();

private:
    struct Item {
        void* Ptr {};
        size_t size {};
        char Data[512] {};
        std::string Tag {};
    };

    std::vector<Item> m_UndoItems;
    std::vector<Item> m_RedoItems;
    std::vector<Item> m_TempItems;

    std::unordered_map<std::string, Item> m_SingleItems;
};
