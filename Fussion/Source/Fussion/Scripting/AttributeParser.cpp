#include "FussionPCH.h"
#include "AttributeParser.h"
#include "Attributes/EditableAttribute.h"
#include "Attributes/RangeAttribute.h"

namespace Fussion {

auto AttributeParser::parse() -> std::vector<Ptr<Scripting::Attribute>>
{
    return parse_attributes();
}

void AttributeParser::on_error(Token token, std::string const& reason)
{
    LOG_ERRORF("{} at {}:{}", reason, token.cursor().Line, token.cursor().LineOffset);
}

auto AttributeParser::parse_attributes() -> std::vector<Ptr<Scripting::Attribute>>
{
    std::vector<Ptr<Scripting::Attribute>> ret{};
    do {
        ret.push_back(parse_attribute());
    } while (match({ TokenType::Comma }));
    return ret;
}

auto AttributeParser::parse_attribute() -> Ptr<Scripting::Attribute>
{
    consume(TokenType::Identifier, "Expected identifier");
    auto name = std::get<std::string>(previous().value());
    std::vector<Value> values{};
    if (match({ TokenType::LParen })) {
        // Arguments
        if (!check_current_token(TokenType::RParen)) {
            do {
                if (auto value = parse_value()) {
                    values.push_back(*value);
                }
            } while (match({ TokenType::Comma }));
        }

        consume(TokenType::RParen, "Expected ')'.");
    }

    using namespace std::string_literals;
    if (name == "Range"s) {
        auto min = std::get<f32>(values[0]);
        auto max = std::get<f32>(values[1]);
        return make_ptr<Scripting::RangeAttribute>(min, max);
    } else if (name == "Editable") {
        return make_ptr<Scripting::EditableAttribute>();
    }
    return nullptr;
}

auto AttributeParser::parse_boolean(std::string_view str) -> std::optional<bool>
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

auto AttributeParser::parse_value() -> std::optional<Value>
{
    if (match({ TokenType::Number, TokenType::String })) {
        return previous().value();
    }
    if (match({ TokenType::Identifier })) {
        auto value = previous().value();
        if (std::holds_alternative<std::string>(value)) {
            auto name = std::get<std::string>(value);
            if (auto bvalue = parse_boolean(name)) {
                return *bvalue;
            }
            return name;
        }
    }

    on_error(peek(), "Expected some kind of value");
    return {};
}

}
