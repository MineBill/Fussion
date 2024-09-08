#include "FussionPCH.h"
#include "SimpleParser.h"

namespace Fussion {
Token SimpleParser::peek() const {
    return m_tokens.at(m_index);
}

bool SimpleParser::is_at_end() const {
    return peek().type() == TokenType::Eof;
}

Token SimpleParser::previous() const {
    return m_tokens.at(m_index - 1);
}

Token SimpleParser::next() const {
    if (is_at_end())
        return peek();
    return m_tokens.at(m_index + 1);
}

bool SimpleParser::check_current_token(TokenType type) const {
    if (is_at_end())
        return false;
    return peek().type() == type;
}

Token SimpleParser::advance() {
    if (!is_at_end())
        m_index++;
    return previous();
}

bool SimpleParser::match(std::vector<TokenType> const& tokens) {
    for (auto const& type : tokens) {
        if (check_current_token(type)) {
            advance();
            return true;
        }
    }

    return false;
}

bool SimpleParser::match_no_advance(std::vector<TokenType> const& tokens) const
{
    for (auto const& type : tokens) {
        if (check_current_token(type)) {
            return true;
        }
    }

    return false;
}

Token SimpleParser::consume(TokenType type, std::string const& reason) {
    if (check_current_token(type))
        return advance();
    auto token = peek();
    on_error(token, reason);
    return token;
}
}
