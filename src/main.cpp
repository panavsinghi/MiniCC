#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "compile.hpp"
#include "vm.hpp"

int main(int argc, char** argv) {
    bool optimize = true, dump = false, stats = false;
    const char* path = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--no-opt")) optimize = false;
        else if (!std::strcmp(argv[i], "--dump")) dump = true;
        else if (!std::strcmp(argv[i], "--stats")) stats = true;
        else path = argv[i];
    }
    if (!path) {
        std::cerr << "usage: minicc <file> [--no-opt] [--dump] [--stats]\n";
        return 64;
    }
    std::ifstream in(path);
    if (!in) {
        std::cerr << "error: cannot open " << path << '\n';
        return 66;
    }
    std::stringstream buf;
    buf << in.rdbuf();
    try {
        OptStats os;
        Chunk chunk = compile(buf.str(), optimize, &os);
        if (stats) {
            Chunk base = compile(buf.str(), false);
            std::cerr << "instructions: " << base.code.size() << " -> " << chunk.code.size()
                      << " (folded " << os.folded << ", identities " << os.identities
                      << ", branches removed " << os.branches << ")\n";
        }
        if (dump) std::cout << disassemble(chunk);
        else for (long long v : run(chunk).output) std::cout << v << '\n';
    } catch (const CompileError& e) {
        std::cerr << "compile error: " << e.what() << '\n';
        return 1;
    } catch (const RuntimeError& e) {
        std::cerr << "runtime error: " << e.what() << '\n';
        return 2;
    }
    return 0;
}
