#pragma once
#include "Fussion/Core/Types.h"

#include <string>
#include <vector>

namespace Fussion {
    struct Cursor {
        u32 Line { 0 };
        u32 LineOffset { 0 };
    };

    enum class TokenType {
        LParen,
        RParen,
        LBrace,
        RBrace,
        LSquareBracket,
        RSquareBracket,
        Comma,
        Dot,
        Minus,
        Plus,
        Semicolon,
        Slash,
        BackSlash,
        Star,

        Question,
        Colon,
        Bang,
        BangEqual,
        Equal,
        EqualEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,

        Identifier,
        String,
        Number,

        Eof,
    };

    using ValueType = std::variant<std::string, f32, bool>;

    class Token {
    public:
        explicit Token(TokenType type, Cursor cursor = {});
        Token(TokenType type, ValueType const& value, Cursor cursor = {});

        std::string ToString() const;
        [[nodiscard]] TokenType Type() const { return m_Type; }
        [[nodiscard]] ValueType Value() const { return m_Value; }
        [[nodiscard]] Cursor GetCursor() const { return m_Cursor; }

    private:
        Cursor m_Cursor {};
        TokenType m_Type {};
        ValueType m_Value {};
    };

    class SimpleLexer {
    public:
        explicit SimpleLexer(std::string const& source);

        std::vector<Token> Scan();

    private:
        u8 Advance();
        bool Match(u8 ch);
        u8 Peek() const;
        u8 PeekNext() const;
        void PushToken(TokenType type);
        void PushToken(TokenType type, ValueType value);
        void ScanToken();
        bool IsAtEnd() const;
        void ParseString();
        void ParseNumber();
        void ParseIdentifier();

        u32 m_Index { 0 };
        Cursor m_Cursor {};
        std::string m_Source {};
        std::vector<Token> m_Tokens {};
    };
}
