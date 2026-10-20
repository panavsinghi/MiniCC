#pragma once
#include <string>
#include "codegen.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "sema.hpp"

inline Chunk compile(const std::string& source, bool optimize = true, OptStats* stats = nullptr) {
    Parser parser(tokenize(source));
    Program program = parser.parse();
    Sema().check(program);
    if (optimize) {
        OptStats s = Optimizer().run(program);
        if (stats) *stats = s;
    }
    return CodeGen().generate(program);
}
