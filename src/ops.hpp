#pragma once
#include "ast.hpp"

inline bool evalBinary(Op op, long long a, long long b, long long& r) {
    auto ua = static_cast<unsigned long long>(a);
    auto ub = static_cast<unsigned long long>(b);
    switch (op) {
        case Op::Add: r = static_cast<long long>(ua + ub); return true;
        case Op::Sub: r = static_cast<long long>(ua - ub); return true;
        case Op::Mul: r = static_cast<long long>(ua * ub); return true;
        case Op::Div:
            if (b == 0) return false;
            r = b == -1 ? static_cast<long long>(0ULL - ua) : a / b;
            return true;
        case Op::Mod:
            if (b == 0) return false;
            r = b == -1 ? 0 : a % b;
            return true;
        case Op::Eq: r = a == b; return true;
        case Op::Ne: r = a != b; return true;
        case Op::Lt: r = a < b; return true;
        case Op::Le: r = a <= b; return true;
        case Op::Gt: r = a > b; return true;
        case Op::Ge: r = a >= b; return true;
        case Op::And: r = (a != 0) && (b != 0); return true;
        case Op::Or: r = (a != 0) || (b != 0); return true;
        default: return false;
    }
}

inline long long evalUnary(Op op, long long a) {
    if (op == Op::Neg) return static_cast<long long>(0ULL - static_cast<unsigned long long>(a));
    return a == 0;
}
