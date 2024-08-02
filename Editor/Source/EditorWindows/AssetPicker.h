#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/meta.hpp/meta_base.hpp>

class AssetPicker final {
public:
    void Update();

    void Show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type);

private:
    bool m_Show{ false };
    meta_hpp::member m_Member;
    meta_hpp::uvalue m_Instance;
    Fussion::AssetType m_Type{};
    bool m_Opened{};

    std::vector<Fussion::AssetHandle> m_ViableHandles{};
};
