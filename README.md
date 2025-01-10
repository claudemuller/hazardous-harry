# Hazardous Harry

A Dangerous Dave clone in C and SDL.

## Table of Contents

1. [Requirements](#requirements)

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

## Running

```bash
make run
```

## Build a Debug Binary

```bash
make debug-build
```

## Start `lldb` or `gdb` With Debug Binary

```bash
make debug
```

## Cleaning the Project

```bash
make clean
```

## Generate Compilation Database (for auto-completion)

```bash
make gen-compilation-db
```
