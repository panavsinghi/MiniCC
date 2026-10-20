#pragma once
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "ast.hpp"

enum class Ins : uint8_t {
    Push, Load, Store,
    Add, Sub, Mul, Div, Mod, Eq, Ne, Lt, Le, Gt, Ge,
    Neg, Not, Jmp, Jz, Jnz, Print, Halt
};

struct Instr {
    Ins op;
    long long arg = 0;
    int line = 0;
};

struct Chunk {
    std::vector<Instr> code;
    int slots = 0;
};

inline Op binaryOp(Ins i) {
    static const Op table[] = {Op::Add, Op::Sub, Op::Mul, Op::Div, Op::Mod, Op::Eq,
                               Op::Ne, Op::Lt, Op::Le, Op::Gt, Op::Ge};
    return table[static_cast<int>(i) - static_cast<int>(Ins::Add)];
}

inline Ins binaryIns(Op op) {
    static const Ins table[] = {Ins::Add, Ins::Sub, Ins::Mul, Ins::Div, Ins::Mod, Ins::Eq,
                                Ins::Ne, Ins::Lt, Ins::Le, Ins::Gt, Ins::Ge};
    return table[static_cast<int>(op)];
}

inline const char* mnemonic(Ins i) {
    static const char* names[] = {"PUSH", "LOAD", "STORE", "ADD", "SUB", "MUL", "DIV", "MOD",
                                  "EQ", "NE", "LT", "LE", "GT", "GE", "NEG", "NOT",
                                  "JMP", "JZ", "JNZ", "PRINT", "HALT"};
    return names[static_cast<int>(i)];
}

inline std::string disassemble(const Chunk& c) {
    std::ostringstream os;
    for (size_t i = 0; i < c.code.size(); ++i) {
        const Instr& in = c.code[i];
        os << (i < 10 ? "000" : i < 100 ? "00" : i < 1000 ? "0" : "") << i << "  " << mnemonic(in.op);
        if (in.op == Ins::Push || in.op == Ins::Load || in.op == Ins::Store ||
            in.op == Ins::Jmp || in.op == Ins::Jz || in.op == Ins::Jnz)
            os << ' ' << in.arg;
        os << '\n';
    }
    return os.str();
}
