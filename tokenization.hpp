#pragma once
#include <string>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>


enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
};

inline bool is_bin_op(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
        case TokenType::fslash:
        case TokenType::star:
            return true;
        default:
            return false;


    }
}
inline const char *to_string(const TokenType &type) {
    switch (type) {
        case TokenType::exit:
            return "`exit`";
        case TokenType::int_lit:
            return "`int literal`";
        case TokenType::semi:
            return "`;`";
        case TokenType::open_paren:
            return "`(`";
        case TokenType::close_paren:
            return "`)`";
        case TokenType::ident:
            return "`ident`";
        case TokenType::let:
            return "`let`";
        case TokenType::eq:
            return "`=`";
        case TokenType::plus:
            return "`+`";
        case TokenType::star:
            return "`*`";
        case TokenType::minus:
            return "`-`";
        case TokenType::fslash:
            return "`fslash`";
        case TokenType::open_curly:
            return "`{`";
        case TokenType::close_curly:
            return "`}`";
        case TokenType::if_:
            return "`if`";
        case TokenType::elif:
            return "`elif`";
        case TokenType::else_:
            return "`else`";
    }
    assert(false);
}

inline std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::minus:
        case TokenType::plus:
            return 0;
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    std::optional<std::string> value {};
};

class Tokenizer {
public:
    inline explicit Tokenizer(std::string  src)
        : m_src(std::move(src))
    {}
    inline std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::string buf;
        int line_count = 1;
        while (peek().has_value()) {
            //std::cout << "char  '" << peek().value() << "'\n";
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({TokenType::exit, line_count});
                    buf.clear();
                }
                else if (buf == "let") {
                    tokens.push_back({TokenType::let, line_count});
                    buf.clear();
                }
                else if (buf == "if") {
                    tokens.push_back({TokenType::if_, line_count});
                    buf.clear();
                }
                else if (buf == "elif") {
                    tokens.push_back({TokenType::elif, line_count});
                    buf.clear();
                }
                else if (buf == "else") {
                    tokens.push_back({TokenType::else_, line_count});
                    buf.clear();
                }
                else {
                    tokens.push_back({ TokenType::ident, line_count, buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
               }
                tokens.push_back({ TokenType::int_lit, line_count, buf});
                buf.clear();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
                consume();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({TokenType::open_paren, line_count});
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({TokenType::close_paren, line_count});
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({TokenType::semi, line_count});
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({TokenType::eq, line_count});
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({TokenType::plus, line_count});
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({TokenType::star, line_count});
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({TokenType::minus, line_count});
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({TokenType::fslash, line_count});
            }
            else if (peek().value() == '{') {
                consume();
                tokens.push_back({ TokenType::open_curly, line_count});
            }
            else if (peek().value() == '}') {
                consume();
                tokens.push_back({ TokenType::close_curly, line_count});
            }
            else if (peek().value() == '\n') {
                consume();
                line_count++;
            }
            else if (std::isspace(peek().value())) {
                consume();
            } else {
                std::cerr << "Invalid token" << std::endl;
            }
        }
        return tokens;
    }

private:
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    }
    inline char consume() {
        return m_src.at(m_index++);
    }
    const std::string m_src;
    size_t m_index = 0;
};
