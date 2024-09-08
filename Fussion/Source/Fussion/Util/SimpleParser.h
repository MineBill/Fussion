#pragma once
#include "SimpleLexer.h"

namespace Fussion {
    class SimpleParser {
    public:
        virtual ~SimpleParser() = default;

        using ErrorCallback = std::function<void()>;
        explicit SimpleParser(std::vector<Token> const& tokens): m_tokens(tokens) {}

        virtual void on_error(Token token, std::string const& reason) = 0;

    protected:
        Token peek() const;
        bool is_at_end() const;
        Token previous() const;
        Token next() const;
        bool check_current_token(TokenType type) const;
        Token advance();
        bool match(std::vector<TokenType> const& tokens);
        auto match_no_advance(std::vector<TokenType> const& tokens) const -> bool;
        Token consume(TokenType type, std::string const& reason);

        u32 m_index{ 0 };
        std::vector<Token> m_tokens{};
    };
}
