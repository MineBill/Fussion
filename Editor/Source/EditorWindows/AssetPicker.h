#pragma once
#include <Fussion/Assets/Asset.h>

class AssetPicker final {
public:
    void update();

    void show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type);

private:
    struct Entry {
        Fussion::AssetHandle handle{};
        std::string_view name{};
        bool is_virtual{};
    };

    bool m_show{ false };
    meta_hpp::member m_member;
    meta_hpp::uvalue m_instance;
    Fussion::AssetType m_type{};
    bool m_opened{};

    std::vector<Entry> m_entries{};
};
