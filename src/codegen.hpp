#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "ast.hpp"
#include "bytecode.hpp"

class CodeGen {
public:
    Chunk generate(const Program& p) {
        chunk_ = {};
        scopes_.clear();
        scopes_.emplace_back();
        nextSlot_ = 0;
        for (const auto& s : p) stmt(*s);
        emit(Ins::Halt, 0, 0);
        chunk_.slots = nextSlot_;
        return std::move(chunk_);
    }

private:
    Chunk chunk_;
    std::vector<std::unordered_map<std::string, int>> scopes_;
    int nextSlot_ = 0;

    size_t emit(Ins op, long long arg, int line) {
        chunk_.code.push_back({op, arg, line});
        return chunk_.code.size() - 1;
    }

    void patch(size_t at) { chunk_.code[at].arg = static_cast<long long>(chunk_.code.size()); }

    int lookup(const std::string& n) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto f = it->find(n);
            if (f != it->end()) return f->second;
        }
        return -1;
    }

    void expr(const Expr& e) {
        switch (e.kind) {
            case Expr::Num:
                emit(Ins::Push, e.value, e.line);
                break;
            case Expr::Var:
                emit(Ins::Load, lookup(e.name), e.line);
                break;
            case Expr::Unary:
                expr(*e.lhs);
                emit(e.op == Op::Neg ? Ins::Neg : Ins::Not, 0, e.line);
                break;
            case Expr::Binary:
                if (e.op == Op::And || e.op == Op::Or) {
                    shortCircuit(e);
                } else {
                    expr(*e.lhs);
                    expr(*e.rhs);
                    emit(binaryIns(e.op), 0, e.line);
                }
                break;
        }
    }

    void shortCircuit(const Expr& e) {
        bool isAnd = e.op == Op::And;
        Ins jump = isAnd ? Ins::Jz : Ins::Jnz;
        expr(*e.lhs);
        size_t first = emit(jump, 0, e.line);
        expr(*e.rhs);
        size_t second = emit(jump, 0, e.line);
        emit(Ins::Push, isAnd ? 1 : 0, e.line);
        size_t end = emit(Ins::Jmp, 0, e.line);
        patch(first);
        patch(second);
        emit(Ins::Push, isAnd ? 0 : 1, e.line);
        patch(end);
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
                scopes_.back()[s.name] = nextSlot_;
                emit(Ins::Store, nextSlot_++, s.line);
                break;
            case Stmt::Assign:
                expr(*s.expr);
                emit(Ins::Store, lookup(s.name), s.line);
                break;
            case Stmt::Print:
                expr(*s.expr);
                emit(Ins::Print, 0, s.line);
                break;
            case Stmt::If: {
                expr(*s.expr);
                size_t toElse = emit(Ins::Jz, 0, s.line);
                block(s.body);
                if (s.other.empty()) {
                    patch(toElse);
                } else {
                    size_t toEnd = emit(Ins::Jmp, 0, s.line);
                    patch(toElse);
                    block(s.other);
                    patch(toEnd);
                }
                break;
            }
            case Stmt::While: {
                size_t top = chunk_.code.size();
                expr(*s.expr);
                size_t toEnd = emit(Ins::Jz, 0, s.line);
                block(s.body);
                emit(Ins::Jmp, static_cast<long long>(top), s.line);
                patch(toEnd);
                break;
            }
            case Stmt::Block:
                block(s.body);
                break;
        }
    }
};
