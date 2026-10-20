#pragma once
#include <string>
#include <vector>
#include "bytecode.hpp"
#include "common.hpp"
#include "ops.hpp"

struct RunResult {
    std::vector<long long> output;
    size_t steps = 0;
};

inline RunResult run(const Chunk& c, size_t maxSteps = 10000000) {
    std::vector<long long> stack;
    std::vector<long long> slots(static_cast<size_t>(c.slots), 0);
    RunResult res;
    size_t pc = 0;
    auto pop = [&]() {
        if (stack.empty()) throw RuntimeError("stack underflow");
        long long v = stack.back();
        stack.pop_back();
        return v;
    };
    while (true) {
        if (pc >= c.code.size()) throw RuntimeError("program counter out of range");
        if (++res.steps > maxSteps) throw RuntimeError("step limit exceeded");
        const Instr& in = c.code[pc++];
        switch (in.op) {
            case Ins::Push:
                stack.push_back(in.arg);
                break;
            case Ins::Load:
                stack.push_back(slots[static_cast<size_t>(in.arg)]);
                break;
            case Ins::Store:
                slots[static_cast<size_t>(in.arg)] = pop();
                break;
            case Ins::Neg:
                stack.push_back(evalUnary(Op::Neg, pop()));
                break;
            case Ins::Not:
                stack.push_back(evalUnary(Op::Not, pop()));
                break;
            case Ins::Jmp:
                pc = static_cast<size_t>(in.arg);
                break;
            case Ins::Jz:
                if (pop() == 0) pc = static_cast<size_t>(in.arg);
                break;
            case Ins::Jnz:
                if (pop() != 0) pc = static_cast<size_t>(in.arg);
                break;
            case Ins::Print:
                res.output.push_back(pop());
                break;
            case Ins::Halt:
                return res;
            default: {
                long long b = pop();
                long long a = pop();
                long long r = 0;
                if (!evalBinary(binaryOp(in.op), a, b, r))
                    throw RuntimeError("division by zero (line " + std::to_string(in.line) + ")");
                stack.push_back(r);
                break;
            }
        }
    }
}
