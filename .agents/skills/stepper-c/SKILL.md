---
name: stepper-c
description: "Use when working on the sstep Stepper Motor Control library: TODO the parts (codec, link…), the Python twin, their tests, the examples (bare metal, Zephyr, host serial), MISRA C:2012 compliance, releases, or integrating the library into firmware or host tools."
---

# sstep: Stepper Motor Control library

## Scope

This repository is a portable C11 static library plus a Python twin of the
codec. TODO release state (for example: version 1.0.0 is released; the public API
in `sstep.h` and `inc/` is stable and follows Semantic Versioning). It implements
the Stepper Motor Control **wire format** and a transport-independent link only. Device
models, application logic and transports (UART/USB drivers, threads) belong to the
applications that use it, not here.

| Part | Files | Rule |
| --- | --- | --- |
| Umbrella | `sstep.h` | Includes all public headers; fallback `SSTEP_VERSION_*` |
| Configuration | `inc/sstep_config.h` | Build-time sizes, each overridable with `#ifndef` |
| Codec | `inc/sstep_protocol.h`, `src/sstep_protocol.c` | TODO checksum, parser, encoder |
| Constants | `inc/sstep_defs.h` | Values only from the specification |
| Link | `inc/sstep_link.h`, `src/sstep_link.c` | No OS calls; I/O only through the write callback |
| Python | `tests/integration/sstep/protocol.py` | Standard library only; byte-identical to the C encoder |
| Build | `CMakeLists.txt`, `sstep.cmake`, `cmake/`, `CMakePresets.json` | Library target in `sstep.cmake`; tests and analysis in `cmake/` |
| Examples | `examples/bare_metal`, `examples/host_serial`, `examples/zephyr` | Not library code; built with `SSTEP_EXAMPLES=ON` |
| Tools | TODO `tools/sstep_<tool>` or remove the row | Desktop tool; its native bridge links the library |

## Hard rules

- The library sources include only `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`,
  `<string.h>` and SSTEP headers. No heap, no RTOS, no driver headers, no global
  mutable state (all state lives in caller-owned structs).
- Public symbols use the `sstep_` / `SSTEP_` prefix. Public headers keep the
  `extern "C"` guards and Doxygen comments for every public item.
- Keep the public API stable within a major version. Record every user-visible
  change under `[Unreleased]` in `CHANGELOG.md`.
- The version comes from the `vX.Y.Z` Git tag (`cmake/version.cmake`); keep the
  fallbacks `SSTEP_VERSION_*` and `SSTEP_VERSION` in `sstep.h` equal to
  `SSTEP_DEFAULT_VERSION` there.
- MISRA C:2012: `misra-analyze` must report 0 violations. An unavoidable
  violation needs an entry in the deviation register in `docs/misra.md`. Library
  functions have a single `return` at the end, parenthesize mixed-operator
  expressions, cast ignored return values to `(void)`, keep `++`/`--` as separate
  statements, and index arrays instead of doing pointer arithmetic.
- Every public function validates its pointer arguments and lengths (Dir 4.11).
  Do not add code that cannot be reached; prove bounds from an invariant and
  document it in a comment instead.
- Behavior is defined by the specification (TODO-specification-url). Do not invent values.
- Code must compile without warnings with
  `-std=c11 -Wall -Wextra -Wconversion -Wpedantic`.
- Style: `.clang-format` (4 spaces, Allman braces, 100 columns); file banners and
  section comments as in the existing files (`.vscode/cpp.json.code-snippets`).

## Tests (definition of done)

```sh
cmake --preset "[Debug]Stepper Motor Control" && cmake --build "build/[Debug]Stepper Motor Control"
cmake --preset "[UnitTests]Stepper Motor Control" && cmake --build "build/[UnitTests]Stepper Motor Control" --target coverage
cmake --preset "[IntegrationTests]Stepper Motor Control" && cmake --build "build/[IntegrationTests]Stepper Motor Control" --target integration_tests
cmake --build "build/[Debug]Stepper Motor Control" --target misra-analyze
cmake --preset "[Examples]Stepper Motor Control" && cmake --build "build/[Examples]Stepper Motor Control" && ctest --test-dir "build/[Examples]Stepper Motor Control" -L example
python tools/docs/check_snippets.py
```

The `coverage` target fails below 100 % line or branch coverage of `src/`;
`misra-analyze` fails on any violation. Unit tests are GoogleTest files
`tests/unit/gtest_sstep_*.cpp` (register new files in `sstep.cmake`). The
pytest suite cross-checks the C codec against the Python twin; it is skipped
without `gcc`, so say so when reporting results on such a machine.

Reference vectors (from the specification) that must keep passing: TODO.

## Documentation

MkDocs Material website (`mkdocs.yml`, pages in `docs/`) and a printable PDF manual
built from the same pages (`python tools/docs/build_manual.py`). All documentation
is in English. The documentation tooling is the shared base of the
`sfs-protocol-library` skill: keep `overrides/`, `tools/docs/build_manual.py`,
`manual.css`, `svgkit.py`, `check_snippets.py`, `stack_usage.py` and the stylesheet
identical to the skill's templates and change project data, not structure
(`mkdocs.yml → extra.landing / extra.manual`, `tools/docs/diagrams.py`,
`tools/docs/snippets.json`). Edit the diagram scripts and run them; never edit the
SVG files.

## Releases

1. Set `SSTEP_DEFAULT_VERSION` in `cmake/version.cmake` and the fallbacks in
   `sstep.h` to the new version.
2. Move the `[Unreleased]` entries in `CHANGELOG.md` to `[X.Y.Z] - date`.
3. Merge to `main`, then push the tag `vX.Y.Z`. The Release workflow refuses a tag
   that does not match these files, runs all quality gates and publishes the
   library, headers, MISRA report and PDF manual as a GitHub Release.

Never push a tag or publish a release without the maintainer's explicit request.
