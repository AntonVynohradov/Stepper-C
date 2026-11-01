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
- Optional: GoogleTest and gcovr, Python 3, Cppcheck, clang-format, MkDocs
  (requirements-dev.txt), Doxygen, WeasyPrint with Pango

## Build with presets

| Preset | Purpose |
| --- | --- |
| `SSTEP[Debug]` | Static library with debug information |
| `SSTEP[Release]` | Optimized static library |
| `SSTEP[UnitTests]` | GoogleTest suite, fixed-point backend, with coverage; target `coverage` |
| `SSTEP[UnitTestsFloat]` | The same suite with the float backend |

## Unit tests and coverage

The `coverage` target fails below `SSTEP_COVERAGE_MIN` (100 %) line or branch coverage.

## Static analysis

| Target | Tool |
| --- | --- |
| `cppcheck-analyze` | Cppcheck, all checks |
| `misra-analyze` | Cppcheck MISRA C:2012 addon; fails on any violation |
| `codeql-analyze` | CodeQL `cpp-queries` |

## Formatting

```bash
clang-format -i src/*.c src/*.h inc/*.h sstep.h tests/unit/*.cpp tests/unit/*.hpp
```

## Versioning and releases

Stepper-C follows [Semantic Versioning](https://semver.org): the public API (`sstep.h` and
`inc/`) is stable within a major version. The version comes from the latest `vX.Y.Z` Git
tag (`cmake/version.cmake`), with `SSTEP_VERSION` in `sstep.h` as the fallback for builds
without CMake; every change is recorded under `[Unreleased]` in `CHANGELOG.md`.

A release tag runs `release.yml`: version check, unit tests with the coverage gate, MISRA
gate, release build, and a GitHub Release with the static library, the headers, the MISRA
report and the PDF manual.

## Documentation

| Task | Command | VS Code task |
| --- | --- | --- |
| Live preview | `mkdocs serve` | *Docs: Serve on localhost* |
| Build the website | `mkdocs build` | *Docs: Build site* |
| Build the PDF manual | `python tools/docs/build_manual.py` | *Docs: Build PDF manual* |
| Regenerate diagrams | `python tools/docs/diagrams.py` | *Docs: Generate diagrams* |
