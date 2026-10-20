#pragma once
#include <utility>
#include "ast.hpp"
#include "ops.hpp"

struct OptStats {
    int folded = 0;
    int identities = 0;
    int branches = 0;
    int total() const { return folded + identities + branches; }
};

class Optimizer {
public:
    OptStats run(Program& p) {
        stats_ = {};
        block(p);
        return stats_;
    }

private:
    OptStats stats_;

    static bool isNum(const ExprP& e, long long v) { return e->kind == Expr::Num && e->value == v; }

    void replaceWith(ExprP& e, ExprP& child) {
        ExprP keep = std::move(child);
        e = std::move(keep);
        ++stats_.identities;
    }

    void expr(ExprP& e) {
        if (e->kind == Expr::Unary) {
            expr(e->lhs);
            if (e->lhs->kind == Expr::Num) {
                e = makeNum(evalUnary(e->op, e->lhs->value), e->line);
                ++stats_.folded;
            }
            return;
        }
        if (e->kind != Expr::Binary) return;
        expr(e->lhs);
        expr(e->rhs);
        if (e->op == Op::And && isNum(e->lhs, 0)) {
            e = makeNum(0, e->line);
            ++stats_.folded;
            return;
        }
        if (e->op == Op::Or && e->lhs->kind == Expr::Num && e->lhs->value != 0) {
            e = makeNum(1, e->line);
            ++stats_.folded;
            return;
        }
        if (e->lhs->kind == Expr::Num && e->rhs->kind == Expr::Num) {
            long long v;
            if (evalBinary(e->op, e->lhs->value, e->rhs->value, v)) {
                e = makeNum(v, e->line);
                ++stats_.folded;
            }
            return;
        }
        switch (e->op) {
            case Op::Add:
                if (isNum(e->rhs, 0)) replaceWith(e, e->lhs);
                else if (isNum(e->lhs, 0)) replaceWith(e, e->rhs);
                break;
            case Op::Sub:
                if (isNum(e->rhs, 0)) replaceWith(e, e->lhs);
                break;
            case Op::Mul:
                if (isNum(e->rhs, 1)) replaceWith(e, e->lhs);
                else if (isNum(e->lhs, 1)) replaceWith(e, e->rhs);
                break;
            case Op::Div:
                if (isNum(e->rhs, 1)) replaceWith(e, e->lhs);
                break;
            default:
                break;
        }
    }

    void block(std::vector<StmtP>& b) {
        for (auto& s : b) stmt(*s);
    }

    void stmt(Stmt& s) {
        switch (s.kind) {
            case Stmt::Let:
            case Stmt::Assign:
            case Stmt::Print:
                expr(s.expr);
                break;
            case Stmt::If: {
                expr(s.expr);
                block(s.body);
                block(s.other);
                if (s.expr->kind == Expr::Num) {
                    std::vector<StmtP> chosen = std::move(s.expr->value != 0 ? s.body : s.other);
                    s.kind = Stmt::Block;
                    s.body = std::move(chosen);
                    s.other.clear();
                    s.expr.reset();
                    ++stats_.branches;
                }
                break;
            }
            case Stmt::While:
                expr(s.expr);
                block(s.body);
                if (isNum(s.expr, 0)) {
                    s.kind = Stmt::Block;
                    s.body.clear();
                    s.expr.reset();
                    ++stats_.branches;
                }
                break;
            case Stmt::Block:
                block(s.body);
                break;
        }
    }
};
