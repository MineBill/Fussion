#pragma once
#include <string>
#include <string_view>

namespace Fussion::StringUtils {
    auto remove(std::string const& str, std::string_view what) -> std::string_view;
    auto is_whitespace(std::string_view str) -> bool;
}
