#pragma once
#include <string>
#include <unordered_set>
#include <vector>
#include "ast.hpp"
#include "common.hpp"

class Sema {
public:
    void check(const Program& p) {
        scopes_.emplace_back();
        for (const auto& s : p) stmt(*s);
        scopes_.pop_back();
    }

private:
    std::vector<std::unordered_set<std::string>> scopes_;

    bool declared(const std::string& n) const {
        for (const auto& s : scopes_) if (s.count(n)) return true;
        return false;
    }

    void expr(const Expr& e) {
        if (e.kind == Expr::Var && !declared(e.name))
            throw CompileError("undeclared variable '" + e.name + "'", e.line);
        if (e.lhs) expr(*e.lhs);
        if (e.rhs) expr(*e.rhs);
    }

    void block(const std::vector<StmtP>& b) {
        scopes_.emplace_back();
        for (const auto& s : b) stmt(*s);
        scopes_.pop_back();
    }

    void stmt(const Stmt& s) {
        switch (s.kind) {
            case Stmt::Let:
                expr(*s.expr);
                if (!scopes_.back().insert(s.name).second)
                    throw CompileError("redeclaration of '" + s.name + "'", s.line);
                break;
            case Stmt::Assign:
                if (!declared(s.name))
                    throw CompileError("assignment to undeclared variable '" + s.name + "'", s.line);
                expr(*s.expr);
                break;
            case Stmt::Print:
                expr(*s.expr);
                break;
            case Stmt::If:
                expr(*s.expr);
                block(s.body);
                block(s.other);
                break;
            case Stmt::While:
                expr(*s.expr);
                block(s.body);
                break;
            case Stmt::Block:
                block(s.body);
                break;
        }
    }
};
