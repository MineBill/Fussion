﻿#include "FussionPCH.h"
#include "StringUtils.h"

namespace Fussion::StringUtils {
    auto Remove(std::string const& str, std::string_view what) -> std::string_view
    {
        auto pos = str.find(what);
        if (pos != std::string::npos)
            return std::string_view(str).substr(pos + what.size());
        return std::string_view(str);
    }
}