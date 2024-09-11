#include "Token.h"
#include "Allocators/ArenaAllocator.h"
#include "Fussion/Core/DynamicArray.h"

#include "Fussion/Core/String.h"

using namespace Fussion;

class Scanner {
public:
    void init(String source, mem::Allocator const& allocator = mem::heap_allocator())
    {
        m_source = source;
        m_tokens.init(allocator);
    }

    auto scan_tokens() -> Slice<Token>
    {
        while (!is_at_end()) {
            m_start = m_current;
            scan();
        }
        return m_tokens.leak();
    }

private:
    void scan()
    {
        auto ch = advance();
        switch (ch) {
        case ' ':
        case '\t':
            break;
        case '\n':
            m_position.next_line();
            break;
        case '(':
            push_token(TokenType::LeftParen);
            break;
        case ')':
            push_token(TokenType::RightParen);
            break;
        case '{':
            push_token(TokenType::LeftBrace);
            break;
        case '}':
            push_token(TokenType::RightBrace);
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
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !is_at_end()) {
                    (void)advance();
                }
            } else {
                push_token(TokenType::Slash);
            }
            break;
        case '=':
            if (match('=')) {
                push_token(TokenType::EqualEqual);
            } else {
                push_token(TokenType::Equal);
            }
            break;
        case '!':
            if (match('=')) {
                push_token(TokenType::BangEqual);
            } else {
                push_token(TokenType::Bang);
            }
            break;
        case '<':
            if (match('=')) {
                push_token(TokenType::LessEqual);
            } else {
                push_token(TokenType::Less);
            }
            break;
        case '>':
            if (match('=')) {
                push_token(TokenType::GreaterEqual);
            } else {
                push_token(TokenType::Greater);
            }
            break;
        case '"':
            do_string();
            break;
        default:
            if (is_digit(ch)) {
                do_number();
            } else {
                LOG_ERRORF("Unexpected character '{}' at {}", ch, m_position);
            }
        }
    }

    void do_number()
    {
        while (is_digit(peek())) {
            (void)advance();
        }

        // We don't use match here in case the character after
        // the '.' is not a number.
        if (peek() == '.' && is_digit(peek_next())) {
            (void)advance();
            while (is_digit(peek())) {
                (void)advance();
            }
        }

        String s = m_source.view(m_start, m_current);
        auto number = std::strtod(s.data.ptr, nullptr);
        push_token(TokenType::Number, number);
    }

    void do_string()
    {
        while (peek() != '"' && !is_at_end()) {
            if (peek_next() == '\n') {
                m_position.next_line();
            }
            (void)advance();
        }

        if (is_at_end()) {
            LOG_ERRORF("Unterminated string");
            return;
        }

        (void)advance();

        String s = m_source.view(m_start + 1, m_current - 1);
        push_token(TokenType::String, s);
    }

    void push_token(TokenType type)
    {
        push_token(type, {});
    }

    void push_token(TokenType type, Object const& obj)
    {
        m_tokens.append(Token{
            .type = type,
            .file_position = m_position,
            .lexeme = m_source.view(m_start, m_current),
            .literal = obj,
        });
    }

    bool is_digit(char ch)
    {
        return '0' <= ch && ch <= '9';
    }

    bool is_at_end() const
    {
        return m_current >= m_source.len();
    }

    char advance()
    {
        m_position.column++;
        return m_source[m_current++];
    }

    bool match(char ch)
    {
        if (is_at_end())
            return false;
        if (peek() != ch)
            return false;
        (void)advance();
        return true;
    }

    char peek() const
    {
        if (is_at_end())
            return 0;
        return m_source[m_current];
    }

    char peek_next() const
    {
        if (m_current + 1 >= m_source.len())
            return 0;
        return m_source[m_current + 1];
    }

    String m_source{};
    DynamicArray<Token> m_tokens{};

    usz m_start{};
    usz m_current{};
    Position m_position{};
};

int main()
{
    ArenaAllocator arena = ArenaAllocator::create(mem::heap_allocator(), 10'000);
    auto allocator = arena.allocator();

    Scanner scanner;
    scanner.init("{}23.2!21()\"pepegas\"", allocator);

    for (auto const& token : scanner.scan_tokens()) {
        LOG_INFOF("token: {}", token);
    }
}
