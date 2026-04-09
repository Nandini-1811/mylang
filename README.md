# HelloWorld Compiler

A compiler for HelloWorld — a custom educational programming language,
built from scratch in C.

## About the Language

An educational general-purpose language designed so that any beginner
can read a program and understand it instantly — no documentation needed.

Source files use the `.learn` extension.

## Quick Example

```bash
note: fibonacci in HelloWorld
define fib(n is num) returns num 
{
    check (n <= 1) {
    give n
    } otherwise {
    give fib(n - 1) + fib(n - 2)
    }
}
let result is num = fib(10)
show(result)
```

## Build Stages

| Stage | Status | Description |
|-------|--------|-------------|
| Stage 1 | Complete | Scanner — tokenizes .learn source files into token stream |
| Stage 2 | Complete | Parser — builds AST from token stream using recursive descent |
| Stage 3 | In Progress | Semantic Analyzer — type checking and symbol table |
| Stage 4 | Planned | IR Generator — intermediate representation |
| Stage 5 | Planned | Code Generator — executable output |

## Build

```bash
gcc -Wall -Iinclude src/scanner.c src/parser.c src/main.c -o lang
```

## Run

```bash
lang.exe yourfile.learn
```

## Project Structure
```bash
HelloWorld/
├── include/
│   ├── tokens.h       # Token type definitions
│   └── ast.h          # AST node type definitions
├── src/
│   ├── scanner.c      # Stage 1 — Lexical analyzer
│   ├── parser.c       # Stage 2 — Recursive descent parser
│   └── main.c         # Entry point
├── test/
│   ├── test1.learn    # Basic variables and functions
│   ├── test2.learn    # Operators, decimals, loops
│   └── test3.learn    # Comments
└── README.md
```

## Language Features

| Feature | Syntax |
|---|---|
| Variable | `let x is num = 10` |
| Function | `define add(a is num, b is num) returns num { }` |
| Return | `give a + b` |
| Print | `show(x)` |
| If / Else | `check (x > 5) { } otherwise { }` |
| Loop | `repeat (let i is num = 0 ; i < 10 ; i++) { }` |
| Comment | `note: this is a comment` |
| Types | `num, decimal, text, bool, nothing` |

## Author

Built by Nandini Goel — fresher CS project demonstrating compiler design fundamentals.