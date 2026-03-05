# 42sh

## Sumary
A shell project written in C.

This project builds a small POSIX-like shell with its own lexer/parser, AST, execution engine, redirections, expansions and a set of builtins.

## Overview

The program reads commands from an input source (stdin for the main shell, or a stream for subshell/command substitution), parses them into an **AST**, then executes the AST while maintaining a **shell state** (variables, last exit code, running flag, etc.).

Main modules you will find in the codebase:
- **Lexer / tokens**: turns the input stream into tokens
- **Parser**: recursive descent parser producing an AST
- **AST**: data structures representing shell grammar
- **Execution**: walks the AST and runs commands / pipelines / control structures
- **Expansion**: variable expansion, subshell expansion, field splitting using `IFS`
- **Variables**: shell variables + export to environment (`envp`)
- **IO**: input sources (stdio or character stream)

## Features

- Parsing into an AST
- Execution of:
  - command lists
  - pipelines (`|`)
  - logical operators (`&&`, `||`)
  - control structures (`if`, `for`, `while`, `until`, `case`)
- Redirections (`>`, `<`, `>>`, `<<`, `>&`, `<&`, etc.)
- Expansions:
  - variable expansion
  - subshell expansion (command substitution)
  - field splitting (`IFS`)
- Variables management and environment export
- Builtins:
  - `true`, `false`, `echo`, `exit`

## Build

```sh
./configure
make
```

## Run

```sh
./42sh
```

If you want to run it on a script/pipe:

```sh
echo 'echo hello' | ./42sh
```

## Tests

If tests are present in the repository:

```sh
make check
# or
./tests/run_tests.sh
```

## Documentation (Doxygen)

Generate the documentation:

```sh
doxygen Doxyfile
```

Open it:

```sh
xdg-open html/index.html
```

If your `Doxyfile` uses `OUTPUT_DIRECTORY = docs`, open:

```sh
xdg-open docs/html/index.html
```

## Authors

Project made by:
- Mael Minard
- Clement Vabre
- Alexandre Coene
- Jaisy Brahimi

## Image
<img width="777" height="676" alt="image" src="https://github.com/user-attachments/assets/cc135c34-9e7c-4deb-b05c-4ccba0ee5359" />
