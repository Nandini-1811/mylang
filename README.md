# [Language Name] Compiler

A compiler for a custom educational programming language, built from scratch in C.

## About the Language

An educational general-purpose language designed so that any beginner 
can read a program and understand it instantly — no documentation needed.

Source files use the `.learn` extension.

## Quick Example

\```
note: fibonacci in the language
define fib(n is num) returns num {
    check (n <= 1) {
        give n
    } otherwise {
        give fib(n - 1) + fib(n - 2)
    }
}

let result is num = fib(10)
show(result)
\```

## Build Stages

| Stage | Status | Description |
|-------|--------|-------------|
| Stage 1 | Complete | Scanner — tokenizes .learn source files |
| Stage 2 | In Progress | Parser — builds AST from token stream |
| Stage 3 | Planned | Semantic Analyzer |
| Stage 4 | Planned | IR Generator |
| Stage 5 | Planned | Code Generator |

## Build

\```bash
gcc -Wall -Iinclude src/scanner.c src/main.c -o lang
\```

## Run

\```bash
lang.exe yourfile.learn
\```

## Project Structure

\```
mylang/
├── include/
│   └── tokens.h       # Token type definitions
├── src/
│   ├── scanner.c      # Stage 1 — Lexical analyzer
│   └── main.c         # Entry point
├── test/
│   ├── test1.learn    # Basic variables and functions
│   ├── test2.learn    # Operators, decimals, loops
│   └── test3.learn    # Comments
└── README.md
\```

## Author

Built by Nandini Goel — fresher CS project demonstrating compiler design fundamentals.
