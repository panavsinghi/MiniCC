# MiniCC

A small compiler and stack-based virtual machine for a tiny integer language, written in C++17 with no dependencies.

Pipeline: lexer, recursive-descent parser (AST), semantic checks, optimizer, bytecode generator, VM.

## Language

- 64-bit signed integers only, with wrapping arithmetic
- `let x = expr;`, `x = expr;`, `print expr;`
- `if (cond) { } else if (cond) { } else { }`, `while (cond) { }`, nested `{ }` scopes with shadowing
- Operators by precedence: `||`, `&&`, `== !=`, `< <= > >=`, `+ -`, `* / %`, unary `- !`
- `&&` and `||` short-circuit
- `#` starts a comment

## Build and run

```
make
./minicc examples/fibonacci.ml
./minicc examples/folding.ml --dump
./minicc examples/folding.ml --stats
./minicc program.ml --no-opt
make test
```

## Design notes

- Compile errors carry source line numbers; runtime errors (division by zero, step limit) exit with code 2.
- Constant evaluation is shared by the optimizer and the VM (`src/ops.hpp`), so folded and executed results cannot diverge.
- Optimizations: constant folding, `x+0`, `0+x`, `x-0`, `x*1`, `1*x`, `x/1` identities, dead `if` branch removal, `while (0)` removal, short-circuit constant folding.
- Division by zero is never folded; it still traps at runtime.
- `INT64_MIN / -1` and signed overflow wrap instead of invoking undefined behavior.

## Testing

`make test` runs 53 unit tests covering the lexer, parser, semantic checks, optimizer, code generator, VM and disassembler. A differential test runs a 19-program corpus with and without optimization and requires identical output. The suite also passes under AddressSanitizer and UndefinedBehaviorSanitizer.

## Example optimization result

`examples/folding.ml`: 49 instructions without optimization, 13 with (5 constant folds, 3 identity removals, 2 dead branches removed).
