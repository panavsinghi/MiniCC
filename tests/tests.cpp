#include <climits>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "compile.hpp"
#include "vm.hpp"

struct Case {
    const char* name;
    std::function<void()> fn;
};

static std::vector<Case>& registry() {
    static std::vector<Case> r;
    return r;
}

struct Reg {
    Reg(const char* n, std::function<void()> f) { registry().push_back({n, std::move(f)}); }
};

#define TEST(name) \
    static void name(); \
    static Reg reg_##name(#name, name); \
    static void name()

#define CHECK(cond) \
    do { if (!(cond)) throw std::runtime_error(std::string("CHECK failed: " #cond " (line ") + std::to_string(__LINE__) + ")"); } while (0)

#define CHECK_THROWS(expr, Type) \
    do { bool ok = false; try { expr; } catch (const Type&) { ok = true; } \
         if (!ok) throw std::runtime_error(std::string("expected " #Type " from: " #expr " (line ") + std::to_string(__LINE__) + ")"); } while (0)

using Out = std::vector<long long>;

static Out exec(const std::string& src, bool opt) { return run(compile(src, opt)).output; }

static void expectOut(const std::string& src, const Out& want) {
    CHECK(exec(src, false) == want);
    CHECK(exec(src, true) == want);
}

static size_t size(const std::string& src, bool opt) { return compile(src, opt).code.size(); }

static const std::vector<std::string>& corpus() {
    static const std::vector<std::string> c = {
        "print 1 + 2 * 3;",
        "print (1 + 2) * 3 - 4 / 2 % 3;",
        "let a = 0; let b = 1; let i = 0; while (i < 15) { let t = a + b; a = b; b = t; i = i + 1; } print a;",
        "let n = 10; let f = 1; while (n > 1) { f = f * n; n = n - 1; } print f;",
        "let s = 0; let i = 0; while (i <= 10) { if (i % 2 == 0) { s = s + i; } i = i + 1; } print s;",
        "let a = 48; let b = 18; while (b != 0) { let t = b; b = a % b; a = t; } print a;",
        "let x = 7; print x + 0; print 0 + x; print x * 1; print 1 * x; print x - 0; print x / 1;",
        "if (0) { print 1; } else { print 2; } if (1) { print 3; } else { print 4; }",
        "while (0) { print 99; } print 5;",
        "print 0 && 1 / 0; print 1 || 1 / 0; print 5 && 3; print 0 || 0;",
        "let x = 3; if (x < 2) { print 1; } else if (x < 4) { print 2; } else { print 3; }",
        "print -(-5) + !0 + !7;",
        "let x = 1; { let x = 2; print x; } print x;",
        "print 2 * 3 + 4 * 5 - 6 / 3;",
        "let a = 10; let b = 3; print a / b; print a % b; print -a / b; print -a % b;",
        "let i = 0; while (i < 3) { let j = 0; while (j < 3) { print i * 3 + j; j = j + 1; } i = i + 1; }",
        "print 9223372036854775807 + 1;",
        "print (0 - 9223372036854775807 - 1) / -1;",
        "print 1 < 2 == 1; print 3 >= 3; print 2 != 2;"
    };
    return c;
}

TEST(lexer_numbers_idents_keywords) {
    auto t = tokenize("let x1 = 42;");
    CHECK(t.size() == 6);
    CHECK(t[0].kind == T::Let);
    CHECK(t[1].kind == T::Ident && t[1].text == "x1");
    CHECK(t[3].kind == T::Num && t[3].value == 42);
    CHECK(t.back().kind == T::End);
}

TEST(lexer_two_char_operators) {
    auto t = tokenize("== != <= >= && ||");
    CHECK(t[0].kind == T::Eq && t[1].kind == T::Ne && t[2].kind == T::Le);
    CHECK(t[3].kind == T::Ge && t[4].kind == T::And && t[5].kind == T::Or);
}

TEST(lexer_comments_and_line_tracking) {
    auto t = tokenize("# note\nlet\n\nx");
    CHECK(t[0].line == 2);
    CHECK(t[1].line == 4);
}

TEST(lexer_rejects_unexpected_character) { CHECK_THROWS(tokenize("let x = @;"), CompileError); }

TEST(lexer_rejects_oversized_literal) { CHECK_THROWS(tokenize("print 9223372036854775808;"), CompileError); }

TEST(lexer_accepts_max_literal) { CHECK(tokenize("9223372036854775807")[0].value == LLONG_MAX); }

TEST(parser_operator_precedence) { expectOut("print 1 + 2 * 3;", {7}); }

TEST(parser_left_associativity) { expectOut("print 10 - 3 - 2; print 100 / 10 / 5;", {5, 2}); }

TEST(parser_parentheses) { expectOut("print (1 + 2) * 3;", {9}); }

TEST(parser_unary_operators) { expectOut("print -2 * -3; print !0; print !5; print - -4;", {6, 1, 0, 4}); }

TEST(parser_relational_binds_tighter_than_equality) { expectOut("print 1 < 2 == 1;", {1}); }

TEST(parser_else_if_chain) {
    expectOut("let x = 3; if (x < 2) { print 1; } else if (x < 4) { print 2; } else { print 3; }", {2});
}

TEST(parser_missing_semicolon) { CHECK_THROWS(compile("print 1"), CompileError); }

TEST(parser_unbalanced_paren) { CHECK_THROWS(compile("print (1 + 2;"), CompileError); }

TEST(parser_unterminated_block) { CHECK_THROWS(compile("while (1) { print 1;"), CompileError); }

TEST(parser_unexpected_statement_token) { CHECK_THROWS(compile("+ 1;"), CompileError); }

TEST(parser_expected_expression) { CHECK_THROWS(compile("print ;"), CompileError); }

TEST(sema_undeclared_read) { CHECK_THROWS(compile("print y;"), CompileError); }

TEST(sema_undeclared_assignment) { CHECK_THROWS(compile("y = 1;"), CompileError); }

TEST(sema_redeclaration_in_same_scope) { CHECK_THROWS(compile("let a = 1; let a = 2;"), CompileError); }

TEST(sema_shadowing_in_inner_scope) { expectOut("let x = 1; { let x = 2; print x; } print x;", {2, 1}); }

TEST(sema_variable_out_of_scope) { CHECK_THROWS(compile("{ let a = 1; } print a;"), CompileError); }

TEST(sema_self_reference_in_initializer) { CHECK_THROWS(compile("let x = x;"), CompileError); }

TEST(sema_error_reports_line_number) {
    try {
        compile("let a = 1;\nlet b = 2;\nprint c;");
        CHECK(false);
    } catch (const CompileError& e) {
        CHECK(e.line == 3);
    }
}

TEST(vm_fibonacci) {
    expectOut("let a = 0; let b = 1; let i = 0; while (i < 15) { let t = a + b; a = b; b = t; i = i + 1; } print a;", {610});
}

TEST(vm_factorial) { expectOut("let n = 10; let f = 1; while (n > 1) { f = f * n; n = n - 1; } print f;", {3628800}); }

TEST(vm_sum_of_evens) {
    expectOut("let s = 0; let i = 0; while (i <= 10) { if (i % 2 == 0) { s = s + i; } i = i + 1; } print s;", {30});
}

TEST(vm_gcd) { expectOut("let a = 48; let b = 18; while (b != 0) { let t = b; b = a % b; a = t; } print a;", {6}); }

TEST(vm_nested_loops) {
    expectOut("let i = 0; while (i < 2) { let j = 0; while (j < 2) { print i * 2 + j; j = j + 1; } i = i + 1; }", {0, 1, 2, 3});
}

TEST(vm_sibling_scopes_use_distinct_slots) {
    expectOut("{ let a = 1; print a; } { let b = 2; print b; }", {1, 2});
}

TEST(vm_assignment_updates_outer_variable) { expectOut("let x = 1; { x = 5; } print x;", {5}); }

TEST(vm_and_short_circuits_division) { expectOut("print 0 && 1 / 0;", {0}); }

TEST(vm_or_short_circuits_division) { expectOut("print 1 || 1 / 0;", {1}); }

TEST(vm_logical_results_are_boolean) { expectOut("print 5 && 3; print 0 || 7; print 0 || 0;", {1, 1, 0}); }

TEST(vm_division_and_modulo_truncate_toward_zero) { expectOut("print -7 / 2; print -7 % 3; print 7 % -3;", {-3, -1, 1}); }

TEST(vm_signed_overflow_wraps) { expectOut("print 9223372036854775807 + 1;", {LLONG_MIN}); }

TEST(vm_min_divided_by_minus_one_wraps) {
    expectOut("print (0 - 9223372036854775807 - 1) / -1;", {LLONG_MIN});
    expectOut("print (0 - 9223372036854775807 - 1) % -1;", {0});
}

TEST(vm_runtime_division_by_zero) { CHECK_THROWS(run(compile("let z = 0; print 1 / z;")), RuntimeError); }

TEST(vm_constant_division_by_zero_compiles_then_traps) {
    Chunk c = compile("print 1 / 0;");
    CHECK_THROWS(run(c), RuntimeError);
}

TEST(vm_step_limit_stops_infinite_loop) {
    Chunk c = compile("while (1) { }");
    CHECK_THROWS(run(c, 1000), RuntimeError);
}

TEST(vm_output_order) { expectOut("print 3; print 1; print 2;", {3, 1, 2}); }

TEST(optimizer_folds_constant_expression_to_push) {
    CHECK(size("print 2 * 3 + 4;", true) == 3);
    CHECK(size("print 2 * 3 + 4;", false) == 7);
}

TEST(optimizer_removes_additive_and_multiplicative_identities) {
    std::string src = "let x = 7; print x + 0; print 0 + x; print x * 1; print 1 * x; print x - 0; print x / 1;";
    CHECK(size(src, true) < size(src, false));
    expectOut(src, {7, 7, 7, 7, 7, 7});
}

TEST(optimizer_does_not_fold_division_by_zero) {
    OptStats s;
    compile("print 1 / 0;", true, &s);
    CHECK(s.folded == 0);
}

TEST(optimizer_eliminates_dead_if_branch) {
    std::string src = "if (0) { print 1; print 2; print 3; } else { print 9; }";
    CHECK(size(src, true) < size(src, false));
    expectOut(src, {9});
}

TEST(optimizer_takes_constant_true_branch) { expectOut("if (2 > 1) { print 1; } else { print 2; }", {1}); }

TEST(optimizer_removes_while_zero) {
    std::string src = "while (0) { print 99; } print 5;";
    CHECK(size(src, true) < size(src, false));
    expectOut(src, {5});
}

TEST(optimizer_short_circuit_constant_folding) {
    CHECK(size("print 0 && 1 / 0;", true) == 3);
    CHECK(size("print 1 || 1 / 0;", true) == 3);
}

TEST(optimizer_reports_statistics) {
    OptStats s;
    compile("print 1 + 2; if (0) { print 1; }", true, &s);
    CHECK(s.folded >= 1);
    CHECK(s.branches == 1);
    CHECK(s.total() == s.folded + s.identities + s.branches);
}

TEST(optimizer_never_grows_code) {
    for (const auto& p : corpus()) CHECK(size(p, true) <= size(p, false));
}

TEST(optimizer_differential_equivalence_on_corpus) {
    for (const auto& p : corpus()) CHECK(exec(p, true) == exec(p, false));
}

TEST(disassembler_lists_mnemonics_and_halts) {
    std::string d = disassemble(compile("print 1 + 2;", false));
    CHECK(d.find("PUSH 1") != std::string::npos);
    CHECK(d.find("ADD") != std::string::npos);
    CHECK(d.find("PRINT") != std::string::npos);
    CHECK(d.rfind("HALT") != std::string::npos);
}

TEST(codegen_jump_targets_are_in_range) {
    for (const auto& p : corpus()) {
        Chunk c = compile(p, false);
        for (const auto& in : c.code) {
            if (in.op == Ins::Jmp || in.op == Ins::Jz || in.op == Ins::Jnz)
                CHECK(in.arg >= 0 && static_cast<size_t>(in.arg) < c.code.size());
        }
        CHECK(c.code.back().op == Ins::Halt);
    }
}

int main() {
    int failed = 0;
    for (const auto& c : registry()) {
        try {
            c.fn();
            std::cout << "PASS " << c.name << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "FAIL " << c.name << ": " << e.what() << '\n';
        }
    }
    std::cout << (registry().size() - failed) << "/" << registry().size() << " tests passed\n";
    return failed ? 1 : 0;
}
