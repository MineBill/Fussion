#include "FussionPCH.h"
#include "SimpleLexer.h"

#include "Fussion/Log/Log.h"

#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    Token::Token(TokenType type, Cursor cursor)
        : m_Cursor(cursor)
        , m_Type(type)
    { }

    Token::Token(TokenType type, ValueType const& value, Cursor cursor)
        : m_Cursor(cursor)
        , m_Type(type)
        , m_Value(value)
    { }

    std::string Token::ToString() const
    {
        return std::format("{}", magic_enum::enum_name(m_Type));
    }

    SimpleLexer::SimpleLexer(std::string const& source)
        : m_Source(source)
    { }

    std::vector<Token> SimpleLexer::Scan()
    {
        while (!IsAtEnd()) {
            ScanToken();
        }

        m_Tokens.emplace_back(TokenType::Eof);
        return std::move(m_Tokens);
    }

    u8 SimpleLexer::Advance()
    {
        return m_Source.at(m_Index++);
    }

    bool SimpleLexer::Match(u8 ch)
    {
        if (IsAtEnd())
            return false;
        if (m_Source.at(m_Index) != ch)
            return false;
        Advance();
        return true;
    }

    u8 SimpleLexer::Peek() const
    {
        if (IsAtEnd())
            return '\0';
        return m_Source.at(m_Index);
    }

    u8 SimpleLexer::PeekNext() const
    {
        if (m_Index + 1 > m_Source.size())
            return '\0';
        return m_Source.at(m_Index + 1);
    }

    void SimpleLexer::PushToken(TokenType type)
    {
        m_Tokens.emplace_back(type, m_Cursor);
    }

    void SimpleLexer::PushToken(TokenType type, ValueType value)
    {
        m_Tokens.emplace_back(type, value, m_Cursor);
    }

    void SimpleLexer::ScanToken()
    {
        auto ch = Advance();
        switch (ch) {
        case '(':
            PushToken(TokenType::LParen);
            break;
        case ')':
            PushToken(TokenType::RParen);
            break;
        case '{':
            PushToken(TokenType::LBrace);
            break;
        case '}':
            PushToken(TokenType::RBrace);
            break;
        case '[':
            PushToken(TokenType::LSquareBracket);
            break;
        case ']':
            PushToken(TokenType::RSquareBracket);
            break;
        case ',':
            PushToken(TokenType::Comma);
            break;
        case '.':
            PushToken(TokenType::Dot);
            break;
        case '-':
            PushToken(TokenType::Minus);
            break;
        case '+':
            PushToken(TokenType::Plus);
            break;
        case ';':
            PushToken(TokenType::Semicolon);
            break;
        case '*':
            PushToken(TokenType::Star);
            break;
        case '?':
            PushToken(TokenType::Question);
            break;
        case ':':
            PushToken(TokenType::Colon);
            break;
        case '!':
            PushToken(Match('=') ? TokenType::BangEqual : TokenType::Bang);
            break;
        case '>':
            PushToken(Match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            break;
        case '<':
            PushToken(Match('=') ? TokenType::LessEqual : TokenType::Less);
            break;
        case '=':
            PushToken(Match('=') ? TokenType::EqualEqual : TokenType::Equal);
            break;
        case '/':
            if (Match('/')) {
                while (!IsAtEnd() && Peek() != '\n')
                    Advance();
            } else {
                PushToken(TokenType::Slash);
            }
            break;
        case '\\':
            PushToken(TokenType::BackSlash);
            break;
        case '"':
            ParseString();
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
                ParseNumber();
            } else if (ch == '_' || std::isalnum(ch) != 0) {
                ParseIdentifier();
            } else {
                LOG_ERRORF("Lexer error at {}:{} : Unexpected character '{:c}'", m_Cursor.Line, m_Cursor.LineOffset, ch);
            }
            break;
        }
        m_Cursor.LineOffset++;
    }

    bool SimpleLexer::IsAtEnd() const
    {
        return m_Index >= m_Source.size();
    }

    void SimpleLexer::ParseString()
    {
        std::string str {};
        while (!IsAtEnd() && Peek() != '"') {
            if (Peek() == '\n') {
                m_Cursor.Line++;
                m_Cursor.LineOffset = 0;
            }
            u8 ch = Advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }
        Advance();
        PushToken(TokenType::String, std::string(str));
    }

    void SimpleLexer::ParseNumber()
    {
        std::string str { m_Source.at(m_Index - 1) };
        while (!IsAtEnd() && std::isdigit(Peek()) != 0) {
            u8 ch = Advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }

        if (Peek() == '.' && std::isdigit(PeekNext()) != 0) {
            u8 ch = Advance();
            str += static_cast<s8>(ch);
            while (std::isdigit(Peek()) != 0) {
                u8 chn = Advance();
                str += static_cast<s8>(chn);
                m_Cursor.LineOffset++;
            }
        }

        PushToken(TokenType::Number, static_cast<f32>(std::stod(str)));
    }

    void SimpleLexer::ParseIdentifier()
    {
        std::string str { m_Source.at(m_Index - 1) };
        while (!IsAtEnd() && (std::isalnum(Peek()) != 0 || Peek() == '_')) {
            u8 ch = Advance();
            str += static_cast<s8>(ch);
            m_Cursor.LineOffset++;
        }

        PushToken(TokenType::Identifier, std::string(str));
    }
}
