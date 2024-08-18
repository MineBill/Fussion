#include "FussionPCH.h"
#include "Clap.h"

#include "Log/Log.h"
#include "Util/SimpleLexer.h"
#include "Util/SimpleParser.h"

#include <magic_enum/magic_enum.hpp>

namespace Fussion {

class ArgParser final : public SimpleParser {
public:
    struct Option {
        std::string Name{};
        Clap::Value Value{};
    };

    explicit ArgParser(std::vector<Token> const& tokens)
        : SimpleParser(tokens)
    {
        // for (auto const& token : tokens) {
        //     LOG_INFOF("Token {}", magic_enum::enum_name(token.GetType()));
        // }
    }

    virtual void OnError(Token token, std::string const& reason) override
    {
        LOG_ERRORF("");
    }

    auto ParseOptions() -> std::vector<Option>
    {
        std::vector<Option> options{};
        while (!IsAtEnd()) {
            options.push_back(ParseOption());
        }
        return options;
    }

    auto ParseOption() -> Option
    {
        Option opt;
        if (Match({ TokenType::Minus })) {
            Consume(TokenType::Identifier, "Expected name for the argument");
            opt.Name = std::get<std::string>(Previous().GetValue());
            if (Match({ TokenType::Equal })) {
                opt.Value = ParseValue();
            } else {
                // No value is assumed to be bool.
                opt.Value = true;
            }
        }
        return opt;
    }

    auto ParseValue() -> Clap::Value
    {
        auto ParseBoolean = [](std::string_view str) -> std::optional<bool> {
            using namespace std::string_view_literals;
            if (str == "true"sv || str == "True"sv) {
                return true;
            }
            if (str == "false"sv || str == "False"sv) {
                return false;
            }
            return std::nullopt;
        };

        if (Match({ TokenType::Number })) {
            return std::get<f32>(Previous().GetValue());
        }

        if (Match({ TokenType::String })) {
            return std::get<std::string>(Previous().GetValue());
        }

        if (Match({ TokenType::Identifier })) {
            auto value = Previous().GetValue();
            if (std::holds_alternative<std::string>(value)) {
                auto name = std::get<std::string>(value);
                if (auto bvalue = ParseBoolean(name)) {
                    return *bvalue;
                }

                std::string path = name;
                while (!MatchNoAdvance({ TokenType::Minus, TokenType::Eof }) && !IsAtEnd()) {
                    Advance();
                    switch (Previous().GetType()) {
                    case TokenType::Colon:
                        path += ':';
                        break;
                    case TokenType::Slash:
                        path += '/';
                        break;
                    case TokenType::Dot:
                        path += '.';
                        break;
                    case TokenType::Identifier:
                        path += std::get<std::string>(Previous().GetValue());
                        break;
                    default:
                        break;
                    }
                }

                return path;
            }
        } else if (Match({ TokenType::Dot, TokenType::Slash })) {
            std::string path;
            switch (Previous().GetType()) {
            case TokenType::Dot:
                path = ".";
                break;
            case TokenType::Slash:
                path = "/";
                break;
            default:
                break;
            }

            while (!MatchNoAdvance({ TokenType::Minus, TokenType::Eof }) && !IsAtEnd()) {
                Advance();
                switch (Previous().GetType()) {
                case TokenType::Colon:
                    path += ':';
                    break;
                case TokenType::Slash:
                    path += '/';
                    break;
                case TokenType::Dot:
                    path += '.';
                    break;
                case TokenType::Identifier:
                    path += std::get<std::string>(Previous().GetValue());
                    break;
                default:
                    break;
                }
            }

            return path;
        }
        return {};
    }
};

Clap::Clap(std::string const& input)
{
    Reset(std::move(input));
}

auto Clap::Parse() -> void
{
    SimpleLexer lexer(m_Cli);
    ArgParser parser(lexer.Scan());

    for (auto const& option : parser.ParseOptions()) {
        if (!m_Options.contains(option.Name))
            continue;

        m_ParsedOptions[option.Name] = option.Value;
    }
}

auto Clap::Reset(std::string const& input) -> void
{
    m_ParsedOptions.clear();
    m_Cli = input;
}

auto Clap::ToString(int argc, const char** argv) -> std::string
{
    std::stringstream arguments{};
    for (int i = 1; i < argc; i++) {
        arguments << argv[i] << ' ';
    }
    return arguments.str();
}

// void usage(int argc, char** argv)
// {
//     Clap clap(argc, argv);
//
//     if (auto path = clap.Option<std::string>("Project")) {
//         LoadProject(*path);
//     }
// }

}
