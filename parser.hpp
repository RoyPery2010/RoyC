#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "./tokenization.hpp"
#include "./arena.hpp"
struct NodeTermParen;

struct NodeTermIntLit {
    Token int_lit;
};
struct NodeTermIdent {
    Token ident;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr* expr;
};
struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*, NodeBinExprSub*, NodeBinExprDiv*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};
struct NodeStmtLet {
    Token ident;
    NodeExpr* expr{};
};
struct NodeStmt;
struct NodeScope {
    std::vector<NodeStmt*> stmts;
};
struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr* expr {};
    NodeScope* scope {};
    std::optional<NodeIfPred*> pred;
};
struct NodeIfPredElse {
    NodeScope* scope {};
};
struct NodeIfPred {
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr {};
    NodeScope* scope {};
    std::optional<NodeIfPred*> pred;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr {};
};


struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*, NodeScope*, NodeStmtIf*, NodeStmtAssign*> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
public:
    inline explicit Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens), m_allocator(1024 * 1024 * 4) {
    }

    void error_expected(const std::string& msg) const {
        std::cerr << "[Parse Error] Expected " << msg << " on line " << peek(-1).value().line << std::endl;
        exit(EXIT_FAILURE);
    }
    std::optional<NodeBinExpr*> parse_bin_expr() {
        if (const auto lhs = parse_expr()) {
            auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            if (peek().has_value() && peek().value().type == TokenType::plus) {
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                bin_expr_add->lhs = lhs.value();
                consume();
                if (const auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->var = bin_expr_add;
                    return bin_expr;
                } else {
                    error_expected("expression");
                }
            } else {
                std::cerr << "Unsupported binary operator" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            return {};
        }
    }
    std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;

        } else if (auto ident = try_consume(TokenType::ident)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume(TokenType::close_paren, "`)`");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr(int min_prec = 0) {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();
        while (true) {
            std::optional<Token> curr_tok = peek();
            if (!curr_tok.has_value()) break; // No more tokens
            std::optional<int> prec = bin_prec(curr_tok.value().type);
            if (!prec.has_value()) break; // Not an operator token
            if (prec.value() < min_prec) break; // Operator precedence is too low
            Token op = consume();
            const auto [type, line, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(prec.value() + 1);
            if (!expr_rhs.has_value()) {
                error_expected("expression");
            }

            auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                bin_expr->var = add;
                add->lhs = expr_lhs;
                add->rhs = expr_rhs.value();
            } else if (op.type == TokenType::star) {
                auto multi = m_allocator.alloc<NodeBinExprMulti>();
                bin_expr->var = multi;
                multi->lhs = expr_lhs;
                multi->rhs = expr_rhs.value();
            } else if (op.type == TokenType::minus) {
                auto minus = m_allocator.alloc<NodeBinExprSub>();
                bin_expr->var = minus;
                minus->lhs = expr_lhs;
                minus->rhs = expr_rhs.value();
            } else if (op.type == TokenType::fslash) {
                auto fslash = m_allocator.alloc<NodeBinExprDiv>();
                bin_expr->var = fslash;
                fslash->lhs = expr_lhs;
                fslash->rhs = expr_rhs.value();
            }
            else {
                assert(false);
            }
            auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
            expr_lhs2->var = bin_expr;
            expr_lhs = expr_lhs2;
        }
        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope() {
        if (!try_consume(TokenType::open_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.alloc<NodeScope>();
        while (auto stmt = parse_stmt()) {
            auto stmtInScope = m_allocator.alloc<NodeStmt>();
            *stmtInScope = *stmt;
            scope->stmts.push_back(stmtInScope);
        }
        try_consume(TokenType::close_curly, "`}`");
        return scope;
    }
    std::optional<NodeIfPred*> parse_if_pred() {
        if (try_consume(TokenType::elif)) {
            try_consume(TokenType::open_paren, "`(`");
            const auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::close_paren, "`)`");
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            } else {
                error_expected("scope");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = elif;
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            const auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                error_expected("scope");
            }
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = else_;
            return pred;
        }
        return {};
    }

    std::optional<NodeStmt> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value() && peek(1).value().type == TokenType::open_paren)
        {
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            consume(); // Read exit
            consume(); // Read "("
            if (auto expr = parse_expr()) {
                stmt_exit->expr = { expr.value() };
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::close_paren, "`)`");
            try_consume(TokenType::semi, "`;`");
            return NodeStmt {.var = stmt_exit};
        }
        else if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume(); // Read ident
            consume(); // Read "="
            if (auto expr = parse_expr()) {
                stmt_let->expr = { expr.value() };
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::semi, "`;`");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return *stmt;
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value() && peek(1).value().type == TokenType::eq) {
            const auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::semi, "`;`");
            const auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = assign;
            return *stmt;
        }
        else if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope.value();
                return *stmt;
            } else {
                error_expected("scope");
            }
        }
        else if (auto if_ = try_consume(TokenType::if_)) {
            try_consume(TokenType::open_paren, "`(`");
            auto stmt_if = m_allocator.alloc<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::close_paren, "`)`");
            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("scope");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_if;
            return *stmt;
        }
        return {};

    };
    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            //std::cout << "parse_stmt " << (unsigned) peek().value().type << std::endl;
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                error_expected("statement");
            }
        }
        return prog;
    }
private:
    [[nodiscard]] inline std::optional<Token> peek(int ahead = 0) const
    {
        if (m_index + ahead >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + ahead);
        }
    }
    inline Token consume() {
        return m_tokens.at(m_index++);
    }
    inline Token try_consume(TokenType type, const std::string& msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
    }
    inline std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            return {};
        }
    }
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};