#pragma once
#include "Types.h"
#include <variant>
#include <string>
#include <unordered_map>

namespace Fussion {

class Clap {
public:
    enum class ValueType {
        String,
        Number,
        Boolean,
    };

    using Value = std::variant<std::monostate, std::string, s32, f32, bool>;

    Clap() = default;
    explicit Clap(std::string const& input);

    template<typename T>
    auto Option(std::string_view name) -> void
    {
        OptionType t;
        if constexpr (std::is_same_v<T, std::string>) {
            m_Options[std::string(name)] = { ValueType::String, std::monostate{} };
        } else if constexpr (std::is_same_v<T, f32>) {
            m_Options[std::string(name)] = { ValueType::Number, std::monostate{} };
        } else if constexpr (std::is_same_v<T, bool>) {
            m_Options[std::string(name)] = { ValueType::Boolean, std::monostate{} };
        } else {
            static_assert(false, "Unsupported type for option, only 'std::string', 'f32' and 'bool' are supported");
        }
    }

    template<typename T>
    auto Option(std::string_view name, T default_value) -> void
    {
        OptionType t;
        if constexpr (std::is_same_v<T, std::string>) {
            m_Options[std::string(name)] = { ValueType::String, default_value };
        } else if constexpr (std::is_same_v<T, f32>) {
            m_Options[std::string(name)] = { ValueType::Number, default_value };
        } else if constexpr (std::is_same_v<T, bool>) {
            m_Options[std::string(name)] = { ValueType::Boolean, default_value };
        } else {
            static_assert(false, "Unsupported type for option, only 'std::string', 'f32' and 'bool' are supported");
        }
    }

    template<typename T>
    auto Get(std::string const& name) -> std::optional<T>
    {
        if (!m_Options.contains(name))
            return std::nullopt;
        if (!m_ParsedOptions.contains(name)) {
            if (std::holds_alternative<std::monostate>(m_Options[name].Value)) {
                return std::nullopt;
            }
            return std::get<T>(m_Options[name].Value);
        }

        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, f32> || std::is_same_v<T, bool>) {
            if (!std::holds_alternative<T>(m_ParsedOptions[name]))
                return std::nullopt;
            return std::get<T>(m_ParsedOptions[name]);
        } else {
            static_assert(false, "Unsupported type for option, only 'std::string', 'f32' and 'bool' are supported");
        }
    }

    auto Parse() -> void;

    auto Reset(std::string const& input) -> void;

    static auto ToString(int argc, const char** argv) -> std::string;

private:
    struct OptionType {
        ValueType Type;
        Value Value;
    };

    std::string m_Cli{};
    std::unordered_map<std::string, OptionType> m_Options{};
    std::unordered_map<std::string, Value> m_ParsedOptions{};
};

}
