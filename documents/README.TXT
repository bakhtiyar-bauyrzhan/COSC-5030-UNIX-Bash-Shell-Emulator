# Unix Shell Implementation - Phase 1

## Overview
This is a custom Unix shell implemented in C. It supports a basic REPL, external command execution via fork/exec/wait, argument tokenization, and globbing. 

## Supported Built-in Commands
* cd <path> : Changes the current directory. Defaults to HOME if no path is provided.
* pwd : Prints the working directory.
* exit : Terminates the shell.

## Build Instructions
1. Run `make` in the root directory.
2. The executable will be generated at `bin/myshell`.
3. Run `make clean` to remove object files and binaries.