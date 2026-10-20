#pragma once
#include <stdexcept>
#include <string>

struct CompileError : std::runtime_error {
    int line;
    CompileError(const std::string& msg, int ln)
        : std::runtime_error("line " + std::to_string(ln) + ": " + msg), line(ln) {}
};

struct RuntimeError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
