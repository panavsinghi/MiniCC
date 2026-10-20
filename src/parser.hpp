#pragma once
#include <utility>
#include <vector>
#include "ast.hpp"
#include "lexer.hpp"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : toks_(std::move(tokens)) {}

    Program parse() {
        Program p;
        while (peek().kind != T::End) p.push_back(statement());
        return p;
    }

private:
    using Level = std::vector<std::pair<T, Op>>;

    std::vector<Token> toks_;
    size_t pos_ = 0;

    static const std::vector<Level>& levels() {
        static const std::vector<Level> table = {
            {{T::Or, Op::Or}},
            {{T::And, Op::And}},
            {{T::Eq, Op::Eq}, {T::Ne, Op::Ne}},
            {{T::Lt, Op::Lt}, {T::Le, Op::Le}, {T::Gt, Op::Gt}, {T::Ge, Op::Ge}},
            {{T::Plus, Op::Add}, {T::Minus, Op::Sub}},
            {{T::Star, Op::Mul}, {T::Slash, Op::Div}, {T::Percent, Op::Mod}}};
        return table;
    }

    const Token& peek() const { return toks_[pos_]; }

    Token next() {
        Token t = toks_[pos_];
        if (pos_ + 1 < toks_.size()) ++pos_;
        return t;
    }

    bool accept(T k) {
        if (peek().kind != k) return false;
        next();
        return true;
    }

    Token expect(T k, const char* what) {
        if (peek().kind != k) throw CompileError(std::string("expected ") + what, peek().line);
        return next();
    }

    std::vector<StmtP> block() {
        expect(T::LBrace, "'{'");
        std::vector<StmtP> body;
        while (peek().kind != T::RBrace) {
            if (peek().kind == T::End) throw CompileError("expected '}'", peek().line);
            body.push_back(statement());
        }
        next();
        return body;
    }

    StmtP statement() {
        auto s = std::make_unique<Stmt>();
        s->line = peek().line;
        switch (peek().kind) {
            case T::Let:
                next();
                s->kind = Stmt::Let;
                s->name = expect(T::Ident, "identifier").text;
                expect(T::Assign, "'='");
                s->expr = expression();
                expect(T::Semi, "';'");
                break;
            case T::Ident:
                s->kind = Stmt::Assign;
                s->name = next().text;
                expect(T::Assign, "'='");
                s->expr = expression();
                expect(T::Semi, "';'");
                break;
            case T::Print:
                next();
                s->kind = Stmt::Print;
                s->expr = expression();
                expect(T::Semi, "';'");
                break;
            case T::If:
                next();
                s->kind = Stmt::If;
                expect(T::LParen, "'('");
                s->expr = expression();
                expect(T::RParen, "')'");
                s->body = block();
                if (accept(T::Else)) {
                    if (peek().kind == T::If) s->other.push_back(statement());
                    else s->other = block();
                }
                break;
            case T::While:
                next();
                s->kind = Stmt::While;
                expect(T::LParen, "'('");
                s->expr = expression();
                expect(T::RParen, "')'");
                s->body = block();
                break;
            case T::LBrace:
                s->kind = Stmt::Block;
                s->body = block();
                break;
            default:
                throw CompileError("unexpected token '" + peek().text + "'", peek().line);
        }
        return s;
    }

    ExprP expression() { return level(0); }

    ExprP level(size_t lv) {
        if (lv == levels().size()) return unary();
        ExprP left = level(lv + 1);
        while (true) {
            const Op* found = nullptr;
            for (const auto& entry : levels()[lv]) {
                if (peek().kind == entry.first) { found = &entry.second; break; }
            }
            if (!found) return left;
            Op op = *found;
            int line = next().line;
            ExprP right = level(lv + 1);
            left = makeBinary(op, std::move(left), std::move(right), line);
        }
    }

    ExprP unary() {
        if (peek().kind == T::Minus || peek().kind == T::Not) {
            Token t = next();
            ExprP operand = unary();
            return makeUnary(t.kind == T::Minus ? Op::Neg : Op::Not, std::move(operand), t.line);
        }
        return primary();
    }

    ExprP primary() {
        Token t = peek();
        if (t.kind == T::Num) { next(); return makeNum(t.value, t.line); }
        if (t.kind == T::Ident) { next(); return makeVar(t.text, t.line); }
        if (t.kind == T::LParen) {
            next();
            ExprP e = expression();
            expect(T::RParen, "')'");
            return e;
        }
        throw CompileError("expected expression", t.line);
    }
};
