#include "e5pch.h"
#include "AttributeParser.h"
#include "Attributes/EditableAttribute.h"
#include "Attributes/RangeAttribute.h"

namespace Fussion {

auto AttributeParser::Parse() -> std::vector<Ptr<Scripting::Attribute>>
{
    return ParseAttributes();
}

void AttributeParser::OnError(Token token, std::string const& reason)
{
    LOG_ERRORF("{} at {}:{}", reason, token.GetCursor().Line, token.GetCursor().LineOffset);
}

auto AttributeParser::ParseAttributes() -> std::vector<Ptr<Scripting::Attribute>>
{
    std::vector<Ptr<Scripting::Attribute>> ret{};
    do {
        ret.push_back(ParseAttribute());
    } while (Match({ TokenType::Comma }));
    return ret;
}

auto AttributeParser::ParseAttribute() -> Ptr<Scripting::Attribute>
{
    Consume(TokenType::Identifier, "Expected identifier");
    auto name = std::get<std::string>(Previous().GetValue());
    std::vector<Value> values{};
    if (Match({ TokenType::LParen })) {
        // Arguments
        if (!CheckCurrentToken(TokenType::RParen)) {
            do {
                if (auto value = ParseValue()) {
                    values.push_back(*value);
                }
            } while (Match({ TokenType::Comma }));
        }

        Consume(TokenType::RParen, "Expected ')'.");
    }

    using namespace std::string_literals;
    if (name == "Range"s) {
        auto min = std::get<f32>(values[0]);
        auto max = std::get<f32>(values[1]);
        return MakePtr<Scripting::RangeAttribute>(min, max);
    } else if (name == "Editable") {
        return MakePtr<Scripting::EditableAttribute>();
    }
    return nullptr;
}

auto AttributeParser::ParseBoolean(std::string_view str) -> std::optional<bool>
{
    using namespace std::string_view_literals;
    if (str == "true"sv || str == "True"sv) {
        return true;
    }
    if (str == "false"sv || str == "False"sv) {
        return false;
    }
    return std::nullopt;
}

auto AttributeParser::ParseValue() -> std::optional<Value>
{
    if (Match({ TokenType::Number, TokenType::String })) {
        return Previous().GetValue();
    }
    if (Match({ TokenType::Identifier })) {
        auto value = Previous().GetValue();
        if (std::holds_alternative<std::string>(value)) {
            auto name = std::get<std::string>(value);
            if (auto bvalue = ParseBoolean(name)) {
                return *bvalue;
            }
            return name;
        }
    }

    OnError(Peek(), "Expected some kind of value");
    return {};
}

}
