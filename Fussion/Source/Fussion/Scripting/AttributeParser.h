#pragma once
#include <Fussion/Log/Log.h>
#include <Fussion/Scripting/Attribute.h>
#include <Fussion/Util/SimpleLexer.h>
#include <Fussion/Util/SimpleParser.h>

namespace Fussion {
    class AttributeParser final : public SimpleParser {
    public:
        explicit AttributeParser(std::vector<Token> const& tokens)
            : SimpleParser(tokens)
        { }

        auto Parse() -> std::vector<Ptr<Scripting::Attribute>>;

        virtual void OnError(Token token, std::string const& reason) override;

    private:
        auto ParseAttributes() -> std::vector<Ptr<Scripting::Attribute>>;
        auto ParseAttribute() -> Ptr<Scripting::Attribute>;
        auto ParseBoolean(std::string_view str) -> std::optional<bool>;
        auto ParseValue() -> std::optional<ValueType>;
    };
}
