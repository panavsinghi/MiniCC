#pragma once
#include <memory>
#include <string>
#include <vector>

enum class Op { Add, Sub, Mul, Div, Mod, Eq, Ne, Lt, Le, Gt, Ge, And, Or, Neg, Not };

struct Expr;
using ExprP = std::unique_ptr<Expr>;

struct Expr {
    enum Kind { Num, Var, Unary, Binary } kind = Num;
    long long value = 0;
    std::string name;
    Op op = Op::Add;
    ExprP lhs;
    ExprP rhs;
    int line = 0;
};

struct Stmt;
using StmtP = std::unique_ptr<Stmt>;

struct Stmt {
    enum Kind { Let, Assign, Print, If, While, Block } kind = Block;
    std::string name;
    ExprP expr;
    std::vector<StmtP> body;
    std::vector<StmtP> other;
    int line = 0;
};

using Program = std::vector<StmtP>;

inline ExprP makeNum(long long v, int line = 0) {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Num;
    e->value = v;
    e->line = line;
    return e;
}

inline ExprP makeVar(std::string name, int line) {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Var;
    e->name = std::move(name);
    e->line = line;
    return e;
}

inline ExprP makeUnary(Op op, ExprP operand, int line) {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Unary;
    e->op = op;
    e->lhs = std::move(operand);
    e->line = line;
    return e;
}

inline ExprP makeBinary(Op op, ExprP l, ExprP r, int line) {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Binary;
    e->op = op;
    e->lhs = std::move(l);
    e->rhs = std::move(r);
    e->line = line;
    return e;
}
