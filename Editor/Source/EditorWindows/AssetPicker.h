﻿#pragma once
#include <Fussion/Assets/Asset.h>

/// Generic asset picker.
/// Displays a window of all assignable types for an asset, of which one can be picked and assigned to a member.
class AssetPicker final {
public:
    void Update();

    void Show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type);

private:
    struct Entry {
        Fussion::AssetHandle Handle {};
        std::string_view Name {};
        bool IsVirtual {};
    };

    bool m_Show { false };
    meta_hpp::member m_Member;
    meta_hpp::uvalue m_Instance;
    Fussion::AssetType m_Type {};
    bool m_Opened {};

    std::vector<Entry> m_Entries {};
};
