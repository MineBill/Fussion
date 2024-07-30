#pragma once
#include "SimpleLexer.h"

namespace Fussion {
class SimpleParser {
public:
    virtual ~SimpleParser() = default;

    using ErrorCallback = std::function<void()>;
    explicit SimpleParser(std::vector<Token> const& tokens): m_Tokens(tokens) {}

    virtual void OnError(Token token, std::string const& reason) = 0;

protected:
    Token Peek() const;
    bool IsAtEnd() const;
    Token Previous() const;
    Token Next() const;
    bool CheckCurrentToken(TokenType type) const;
    Token Advance();
    bool Match(std::vector<TokenType> const& tokens);
    bool MatchNoAdvance(std::vector<TokenType> const& tokens) const;
    Token Consume(TokenType type, std::string const& reason);

    u32 m_Index{ 0 };
    std::vector<Token> m_Tokens{};
};
}
