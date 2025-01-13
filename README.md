# Hazardous Harry

A Dangerous Dave clone in C and SDL.

## Table of Contents

1. [Requirements](#requirements)
2. [Using Dangerous Dave Assets](#using-dangerous-dave-assets)
3. [Building](#building)
4. [Running](#running)
5. [Cleaning the Project](#cleaning-the-project)
6. [Generate Compilation Database](#generate-compilation-database)

## Requirements

- [gcc](https://gcc.gnu.org/)
- or [clang](https://clang.llvm.org/)
- [make](https://www.gnu.org/software/make/)
- [SDL2](https://www.libsdl.org/)
- (Optional) [Bear](https://github.com/rizsotto/Bear) - for auto-completion (in the editor)

## Using Dangerous Dave Assets

### Extracting Tiles

```bash
make extract-tiles
```

### Extracting Level Data

```bash
make extract-levels
```

## Building

```bash
make build
```

### Building a Debug Binary

```bash
make debug-build
```

## Running

```bash
make run
```

### Debugging with `lldb` or `gdb`

```bash
make debug
```

## Cleaning the Project

```bash
make clean
```

## Generate Compilation Database

This is for code completion in some editors like Neovim+nvim-cmp.

```bash
make gen-compilation-db
```
