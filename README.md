# JKC to RISC-V C++ Compiler

A from-scratch compiler written in **C++17** that compiles programs written in a custom statically typed language, **JKC**, into executable **RISC-V assembly**.

The project implements the full compilation pipeline, including lexical analysis, recursive-descent parsing, static type checking, custom three-address IR generation and optimization, control-flow analysis, liveness analysis, graph-coloring register allocation, stack spilling, and RISC-V code generation.

The compiler contains **4,100+ lines of C++** across `src/` and `include/`.

## Compiler Pipeline

```text
JKC Source Code
      ↓
    Lexer
      ↓
    Tokens
      ↓
Recursive-Descent Parser
      ↓
     AST
      ↓
Symbol Table + Type Checker
      ↓
Custom Three-Address IR
      ↓
IR Optimization
      ↓
Basic Block Formation
      ↓
Control-Flow Graph (CFG)
      ↓
Liveness Analysis
      ↓
Interference Graph
      ↓
Graph-Coloring Register Allocation
      ↓
RISC-V Code Generation
      ↓
program.s
      ↓
RISC-V Assembler / Linker
      ↓
QEMU Execution
```

## JKC Language

JKC is a small statically typed language designed specifically for this compiler.

### Supported Types

- 64-bit integers (`int`)
- Booleans (`bool`)

### Supported Features

- Variable declarations and assignments
- Integer arithmetic
- Boolean expressions
- Comparison operators
- Logical operators
- `if / else`
- `while` loops
- Functions
- Multiple function arguments
- Function calls
- Recursion
- Single-line comments
- Static type checking

JKC uses several custom keywords and operators:

| JKC | Meaning |
|---|---|
| `send` | return |
| `is` | `==` |
| `is_lt` | `<` |
| `is_lte` | `<=` |
| `is_gt` | `>` |
| `is_gte` | `>=` |
| `AND` | logical AND |
| `OR` | logical OR |
| `NOT` | logical NOT |

### Example

```jkc
fn fact(n: int) -> int {
    if(n is_lte 1) then {
        send 1;
    }
    else {
        send n * fact(n - 1);
    }
}

fn main() -> int {
    send fact(5);
}
```



# How to Build and Run

The easiest way to build and run the full compiler on Windows is through **WSL (Windows Subsystem for Linux)**.

## 1. Install WSL

Open **PowerShell as Administrator**:

```powershell
wsl --install -d Ubuntu
```

Restart your computer if prompted.

After installation, open Ubuntu or enter WSL from PowerShell:

```powershell
wsl
```

---

## 2. Install Required Packages

Inside Ubuntu/WSL:

```bash
sudo apt update
```

Then install the C++ compiler, RISC-V cross-toolchain, QEMU, and Git:

```bash
sudo apt install -y \
    g++ \
    gcc-riscv64-linux-gnu \
    libc6-dev-riscv64-cross \
    qemu-user \
    git
```

You can verify the tools with:

```bash
g++ --version
riscv64-linux-gnu-gcc --version
qemu-riscv64 --version
```

---

## 3. Clone the Repository

```bash
git clone <repository-url>
cd JKCtoRISCV-Compiler
```

If the repository is already stored on your Windows filesystem, WSL exposes the `C:` drive through:

```text
/mnt/c/
```

For example:

```bash
cd /mnt/c/Users/<username>/Documents/JKCtoRISCV-Compiler
```

---

## 4. Build the Compiler

From the project root:

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o main.exe
```

This builds the JKC compiler as:

```text
main.exe
```

---

## 5. Compile a JKC Program

For example:

```bash
./main.exe examples/factorial.jkc
```

The compiler runs the complete pipeline and produces:

```text
program.s
```

which contains the generated RISC-V assembly.

---

## Optional Compiler Output

Print lexer output:

```bash
./main.exe examples/factorial.jkc --lexer
```

Print optimized IR:

```bash
./main.exe examples/factorial.jkc --opt-ir
```

Print generated RISC-V assembly:

```bash
./main.exe examples/factorial.jkc --asm
```

Multiple flags can also be combined:

```bash
./main.exe examples/factorial.jkc --lexer --opt-ir --asm
```

---

## 6. Assemble and Link the Generated RISC-V

After the compiler creates `program.s`:

```bash
riscv64-linux-gnu-gcc -static program.s -o program
```

This converts the generated RISC-V assembly into a RISC-V executable.

---

## 7. Run the Program with QEMU

```bash
qemu-riscv64 ./program
```

JKC programs currently communicate their `main()` result through the process exit code.

Check it using:

```bash
echo $?
```

For the factorial example:

```jkc
send fact(5);
```

the expected result is:

```text
120
```

---

The compiler translates this program through every compiler phase before generating RISC-V assembly.

---
