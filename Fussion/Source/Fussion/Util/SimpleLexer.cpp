#include "FussionPCH.h"
#include "SimpleLexer.h"

#include "Fussion/Log/Log.h"

#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    Token::Token(TokenType type, Cursor cursor): m_cursor(cursor), m_type(type) {}

    Token::Token(TokenType type, Value const& value, Cursor cursor): m_cursor(cursor), m_type(type), m_value(value) {}

    std::string Token::to_string() const
    {
        return std::format("{}", magic_enum::enum_name(m_type));
    }

    SimpleLexer::SimpleLexer(std::string const& source): m_Source(source) {}

    std::vector<Token> SimpleLexer::scan()
    {
        while (!is_at_end()) {
            scan_token();
        }

        m_Tokens.emplace_back(TokenType::Eof);
        return std::move(m_Tokens);
    }

    u8 SimpleLexer::advance()
    {
        return m_Source.at(m_Index++);
    }

    bool SimpleLexer::match(u8 ch)
    {
        if (is_at_end())
            return false;
        if (m_Source.at(m_Index) != ch)
            return false;
        advance();
        return true;
    }

    u8 SimpleLexer::peek() const
    {
        if (is_at_end())
            return '\0';
        return m_Source.at(m_Index);
    }

    u8 SimpleLexer::peek_next() const
    {
        if (m_Index + 1 > m_Source.size())
            return '\0';
        return m_Source.at(m_Index + 1);
    }

    void SimpleLexer::push_token(TokenType type)
    {
        m_Tokens.emplace_back(type, m_Cursor);
    }

    void SimpleLexer::push_token(TokenType type, Value value)
    {
        m_Tokens.emplace_back(type, value, m_Cursor);
    }

    void SimpleLexer::scan_token()
    {
        auto ch = advance();
        switch (ch) {
        case '(':
            push_token(TokenType::LParen);
            break;
        case ')':
            push_token(TokenType::RParen);
            break;
        case '{':
            push_token(TokenType::LBrace);
            break;
        case '}':
            push_token(TokenType::RBrace);
            break;
        case '[':
            push_token(TokenType::LSquareBracket);
            break;
        case ']':
            push_token(TokenType::RSquareBracket);
            break;
        case ',':
            push_token(TokenType::Comma);
            break;
        case '.':
            push_token(TokenType::Dot);
            break;
        case '-':
            push_token(TokenType::Minus);
            break;
        case '+':
            push_token(TokenType::Plus);
            break;
        case ';':
            push_token(TokenType::Semicolon);
            break;
        case '*':
            push_token(TokenType::Star);
            break;
        case '?':
            push_token(TokenType::Question);
            break;
        case ':':
            push_token(TokenType::Colon);
            break;
        case '!':
            push_token(match('=') ? TokenType::BangEqual : TokenType::Bang);
            break;
        case '>':
            push_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            break;
        case '<':
            push_token(match('=') ? TokenType::LessEqual : TokenType::Less);
            break;
        case '=':
            push_token(match('=') ? TokenType::EqualEqual : TokenType::Equal);
            break;
        case '/':
            if (match('/')) {
                while (!is_at_end() && peek() != '\n')
                    advance();
            } else {
                push_token(TokenType::Slash);
            }
            break;
        case '\\':
            push_token(TokenType::BackSlash);
            break;
        case '"':
            parse_string();
            break;
        case '\n':
            m_Cursor.Line++;
            m_Cursor.LineOffset = 0;
            break;
        case ' ':
        case '\t':
        case '\r':
            break;
        default:
            if (std::isdigit(ch) != 0) {
                parse_number();
            } else if (ch == '_' || std::isalnum(ch) != 0) {
                parse_identifier();
            } else {
                LOG_ERRORF("Lexer error at {}:{} : Unexpected character '{:c}'", m_Cursor.Line, m_Cursor.LineOffset, ch);
            }
            break;
        }
        m_Cursor.LineOffset++;
    }

    bool SimpleLexer::is_at_end() const
    {
        return m_Index >= m_Source.size();
    }

    void SimpleLexer::parse_string()
    {
        std::string str{};
        while (!is_at_end() && peek() != '"') {
            if (peek() == '\n') {
                m_Cursor.Line++;
                m_Cursor.LineOffset = 0;
            }
            u8 ch = advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }
        advance();
        push_token(TokenType::String, std::string(str));
    }

    void SimpleLexer::parse_number()
    {
        std::string str{ m_Source.at(m_Index - 1) };
        while (!is_at_end() && std::isdigit(peek()) != 0) {
            u8 ch = advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }

        if (peek() == '.' && std::isdigit(peek_next()) != 0) {
            u8 ch = advance();
            str += static_cast<s8>(ch);
            while (std::isdigit(peek()) != 0) {
                u8 chn = advance();
                str += static_cast<s8>(chn);
                m_Cursor.LineOffset++;
            }
        }

        push_token(TokenType::Number, static_cast<f32>(std::stod(str)));
    }

    void SimpleLexer::parse_identifier()
    {
        std::string str{ m_Source.at(m_Index - 1) };
        while (!is_at_end() && std::isalnum(peek()) != 0 || peek() == '_') {
            u8 ch = advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }

        push_token(TokenType::Identifier, std::string(str));
    }
}
