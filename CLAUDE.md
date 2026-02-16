# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (run from project root)
cmake -B build

# Build
cmake --build build

# Run
./build/MyRaylibProject
```

## Project Overview

This is a C project using raylib for graphics. The project uses CMake as its build system.

**Dependencies:** raylib must be installed on the system (via package manager or from source).
