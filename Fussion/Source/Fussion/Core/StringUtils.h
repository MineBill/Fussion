#pragma once
#include <string>
#include <string_view>

namespace Fussion::StringUtils {
    auto Remove(std::string const& str, std::string_view what) -> std::string_view;
    auto IsWhitespace(std::string_view str) -> bool;
}
