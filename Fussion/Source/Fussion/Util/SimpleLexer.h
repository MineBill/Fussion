#pragma once
#include "Fussion/Core/Types.h"

#include <string>
#include <vector>

namespace Fussion {
    struct Cursor {
        u32 Line{ 0 };
        u32 LineOffset{ 0 };
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

    using Value = std::variant<std::string, f32, bool>;

    class Token {
    public:
        explicit Token(TokenType type, Cursor cursor = {});
        Token(TokenType type, Value const& value, Cursor cursor = {});

        std::string to_string() const;
        [[nodiscard]] TokenType type() const { return m_type; }
        [[nodiscard]] Value value() const { return m_value; }
        [[nodiscard]] Cursor cursor() const { return m_cursor; }

    private:
        Cursor m_cursor{};
        TokenType m_type{};
        Value m_value{};
    };

    class SimpleLexer {
    public:
        explicit SimpleLexer(std::string const& source);

        std::vector<Token> scan();

    private:
        u8 advance();
        bool match(u8 ch);
        u8 peek() const;
        u8 peek_next() const;
        void push_token(TokenType type);
        void push_token(TokenType type, Value value);
        void scan_token();
        bool is_at_end() const;
        void parse_string();
        void parse_number();
        void parse_identifier();

        s32 m_Index{ 0 };
        Cursor m_Cursor{};
        std::string m_Source{};
        std::vector<Token> m_Tokens{};
    };
}
