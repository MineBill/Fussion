#include "FussionPCH.h"
#include "SimpleParser.h"

namespace Fussion {
Token SimpleParser::Peek() const {
    return m_Tokens.at(m_Index);
}

bool SimpleParser::IsAtEnd() const {
    return Peek().Type() == TokenType::Eof;
}

Token SimpleParser::Previous() const {
    return m_Tokens.at(m_Index - 1);
}

Token SimpleParser::Next() const {
    if (IsAtEnd())
        return Peek();
    return m_Tokens.at(m_Index + 1);
}

bool SimpleParser::CheckCurrentToken(TokenType type) const {
    if (IsAtEnd())
        return false;
    return Peek().Type() == type;
}

Token SimpleParser::Advance() {
    if (!IsAtEnd())
        m_Index++;
    return Previous();
}

bool SimpleParser::Match(std::vector<TokenType> const& tokens) {
    for (auto const& type : tokens) {
        if (CheckCurrentToken(type)) {
            Advance();
            return true;
        }
    }

    return false;
}

bool SimpleParser::MatchNoAdvance(std::vector<TokenType> const& tokens) const
{
    for (auto const& type : tokens) {
        if (CheckCurrentToken(type)) {
            return true;
        }
    }

    return false;
}

Token SimpleParser::Consume(TokenType type, std::string const& reason) {
    if (CheckCurrentToken(type))
        return Advance();
    auto token = Peek();
    OnError(token, reason);
    return token;
}
}
