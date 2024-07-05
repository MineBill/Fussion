#pragma once
#include "meta.hpp/meta_detail/base_info.hpp"

namespace Fussion {
class IDrawable {
    META_HPP_ENABLE_POLY_INFO()
public:
    IDrawable() = default;

    virtual ~IDrawable() = default;
    virtual void Draw() = 0;
};
}
