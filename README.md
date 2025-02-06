# LLVM LEG Backend

This repository imlements a backend for the imaginary leg architecture.

## Installation

1. Clone [llvm-project](https://github.com/llvm/llvm-project) version 19.1.7
2. Apply both patch files in order to the base directory
3. link this directory (must be named LEG) into the llvm file tree at `llvm-project/llvm/lib/Target/LEG`
4. create a build directory
5. run cmake from your build directory (suggested commandline: `cmake -DLLVM_ENABLE_PROJECTS="lld;clang;compiler-rt" -DLLVM_TARGETS_TO_BUILD="X86;ARM;RISCV" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="LEG" -DLLVM_ENABLE_ASSERTIONS=On -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=On -G Ninja path/to/llvm-project/llvm`)
6. build

## Usage

There are generally two ways to use any llvm backend. They differ in how arguments are passed to the backend. Using clang target features are generally only available when there is also a matching clang option that sets the aproprite options on the backend, or using `-Xclang -target-feature -Xclang +<feature>`. When using clang to generate llvm IR and then transpile using llc target features can be set directly.

- through clang `clang -target leg foo.c -o bar`
- through clang + llc:
	- `clang -emit-llvm -Xclang -disable-O0-optnone -c foo.c`
	- `llc -march=leg --filetype=obj foo.bc`