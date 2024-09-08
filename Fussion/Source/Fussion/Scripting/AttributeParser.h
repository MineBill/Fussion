#pragma once
#include <Fussion/Log/Log.h>
#include <Fussion/Util/SimpleLexer.h>
#include <Fussion/Util/SimpleParser.h>
#include "Attribute.h"

namespace Fussion {
    class AttributeParser final : public SimpleParser {
    public:
        explicit AttributeParser(std::vector<Token> const& tokens): SimpleParser(tokens) {}

        auto parse() -> std::vector<Ptr<Scripting::Attribute>>;

        virtual void on_error(Token token, std::string const& reason) override;

    private:
        auto parse_attributes() -> std::vector<Ptr<Scripting::Attribute>>;
        auto parse_attribute() -> Ptr<Scripting::Attribute>;
        auto parse_boolean(std::string_view str) -> std::optional<bool>;
        auto parse_value() -> std::optional<Value>;
    };
}
