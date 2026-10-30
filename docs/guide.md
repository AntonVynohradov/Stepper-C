# Getting Started

This chapter shows how to get Stepper-C, add it to your project, and build and test it.

## Get the library

```bash
git clone https://github.com/AntonVynohradov/Stepper-C.git
```

Each [release](https://github.com/AntonVynohradov/Stepper-C/releases) provides the source
code, a prebuilt static library for Linux x86-64 with its headers, the MISRA report and the
PDF user manual.

## Add it to your project

**CMake.**

```cmake
include(path/to/Stepper-C/sstep.cmake)
target_link_libraries(app PRIVATE sstep)
```

**Any other build system.** Compile `src/*.c` and add the repository root and `inc/` to the
include path.

**Prebuilt package.** Link `lib/libsstep.a` and add `include/` to the include path.

## Prerequisites for development

- C11 compiler, CMake 3.20+
- Optional: GoogleTest and gcovr, Python 3 and pytest, Cppcheck, clang-format, MkDocs
  (requirements-dev.txt), Doxygen, WeasyPrint with Pango

## Build with presets

| Preset | Purpose |
| --- | --- |
| `[Debug]Stepper Motor Control` | Static library with debug information |
| `[Release]Stepper Motor Control` | Optimized static library |
| `[UnitTests]Stepper Motor Control` | GoogleTest executable with coverage; target `coverage` |
| `[IntegrationTests]Stepper Motor Control` | pytest suite; target `integration_tests` |
| `[Examples]Stepper Motor Control` | Host examples and the bare-metal simulation test |

## Unit tests and coverage

The `coverage` target fails below `SSTEP_COVERAGE_MIN` (100 %) line or branch coverage.

## Integration tests

TODO the Python twin and the C/Python cross-check.

## Static analysis

| Target | Tool |
| --- | --- |
| `cppcheck-analyze` | Cppcheck, all checks |
| `misra-analyze` | Cppcheck MISRA C:2012 addon; fails on any violation |
| `codeql-analyze` | CodeQL `cpp-queries` |

## Formatting

```bash
clang-format -i src/*.c inc/*.h sstep.h tests/unit/*.cpp examples/*/*.[ch]
```

## Versioning and releases

TODO (see the skill's repository.md → Release).

## Documentation

| Task | Command | VS Code task |
| --- | --- | --- |
| Live preview | `mkdocs serve` | *Docs: Serve on localhost* |
| Build the website | `mkdocs build` | *Docs: Build site* |
| Build the PDF manual | `python tools/docs/build_manual.py` | *Docs: Build PDF manual* |
| Regenerate diagrams | `python tools/docs/diagrams.py` | *Docs: Generate diagrams* |
