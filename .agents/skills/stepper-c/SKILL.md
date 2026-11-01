---
name: stepper-c
description: "Use when working on Stepper-C (sstep), the C11 closed-loop microstepping current-control library for two-phase stepper motors: the control loops (fast/slow), the motion planner, the power stage mapping (full bridge, shared leg), the fixed-point and float numeric backends, the opaque driver instance, the unit and closed-loop tests on the motor model, MISRA C:2012 compliance, the MkDocs/PDF documentation, releases, or integrating the library into MCU firmware (PWM, ADC, interrupts, RTOS)."
---

# Stepper-C (sstep): stepper motor control library

## Scope

A portable C11 static library that drives a two-phase hybrid stepper motor: PI
current control per phase with back-EMF feed-forward and anti-windup, microstep
angle generation, a trapezoidal motion planner, the voltage limit and duty cycles of
two power stage topologies, protection trips and sensorless stall detection (back-EMF
observer, 1.1.0). Version 1.0.0 is released and 1.1.0 is prepared: the
public API (`sstep.h`, `inc/`) is stable within major version 1, so a removed or
changed public symbol, struct member, enum value or return code needs 2.0.0.
Verified on a motor model in both numeric backends; the next steps are
software-in-the-loop with the Scilab model, the B-G431B-ESC1 port and dead-time
compensation.

Not in the library: timer/PWM/ADC drivers, interrupt setup, encoders, homing,
communication, application logic. They belong to the firmware that uses it.

| Part | Files | Rule |
| --- | --- | --- |
| Public API | `sstep.h` | Every public function declaration (15), version fallbacks; includes the public headers |
| Public types | `inc/sstep_ctrl.h`, `inc/sstep_defs.h`, `inc/sstep_config.h` | Configuration, callbacks, sample, status, opaque `struct sstep`; status codes, faults, states; build-time options |
| Control | `src/sstep_ctrl.c` | Config check, derived gains, state machine, fast loop, slow loop, commands |
| Stall detection | `src/sstep_stall.c` | Back-EMF observer (fast loop), decision with learned offset and up/down counter (slow loop, integer mV), status estimates |
| Motion | `src/sstep_motion.c` | Integer trajectory (Q16 microsteps, 64-bit), both backends |
| Power stage | `src/sstep_bridge.c` | Voltage limit per topology, leg compare values |
| Numeric backend | `src/sstep_math.h`, `src/sstep_math.c`, `src/sstep_sin_table.[ch]` | Saturating Q15/Q31 or float operations, gains, sine table |
| Internal | `src/sstep_internal.h` | `struct sstep_ctx`, `sstep_ctx_of()`, internal declarations |
| Sine table | `tools/sstep_gen_sin_table.py` | Generates `src/sstep_sin_table.c`; never edit the table by hand |
| Build | `CMakeLists.txt`, `sstep.cmake`, `cmake/`, `CMakePresets.json` | Library target in `sstep.cmake`; tests and analysis in `cmake/` |
| Tests | `tests/unit/gtest_sstep_*.cpp`, `tests/unit/sstep_test_plant.hpp` | GoogleTest; the plant header holds the motor model (optional detent torque, dead-time error, ADC span, imposed rotor motion, `make_worst_case()`) and the 17HS19/ESC1 configuration |

## Execution model

- `sstep_fast_loop()` runs in the PWM/ADC interrupt at `fast_loop_hz`: ADC scaling,
  trips, angle interpolation, PI, voltage limit, `pwm_write()`. Bounded, short loops
  only.
- `sstep_slow_loop()` and every command run in one task or the main loop at
  `slow_loop_hz` (`fast_loop_hz` is an integer multiple): trajectory, setpoint, hold
  current, bus compensation, state, events.
- The slow loop hands a setpoint to the fast loop through a double buffer
  (`sp[2]`, `sp_active`, `sp_pending`): clear pending, write the free half, set
  pending. Lock-free only because the interrupt preempts the slow loop on one core;
  keep it that way.
- `sstep_report_fault()` may be called from an interrupt. Faults switch the outputs
  off in the detecting context, then call `on_fault()`.

## Hard rules

- Library sources include only `<stddef.h>`, `<stdint.h>`, `<stdbool.h>` and SSTEP
  headers. No heap, no RTOS, no driver headers, no global or static mutable data
  (`.data` and `.bss` stay at 0 B), no recursion.
- **Encapsulation.** `sstep.h` and `inc/` contain the public API only; nothing under
  `src/` is included from a public header. `struct sstep` is opaque
  (`uint64_t opaque[SSTEP_INSTANCE_WORDS]`); the state is `struct sstep_ctx`,
  reached only through `sstep_ctx_of()` / `sstep_ctx_of_const()` (MISRA deviation
  DEV-01). Public functions take `struct sstep* drv` and start with
  `struct sstep_ctx* m = sstep_ctx_of(drv);`, then check `m == NULL`
  (`SSTEP_ERR_PARAM`) and `m->ready != SSTEP_READY` (`SSTEP_ERR_STATE`) before any
  other access: a zeroed instance has an empty configuration and NULL callbacks.
  Only `sstep_init()` sets `ready`, and only when the derivation succeeds.
- **Instance size.** When `struct sstep_ctx` changes, update `SSTEP_STATE_BYTES`
  in `inc/sstep_ctrl.h` for both backends; when `struct sstep_config` changes,
  update `SSTEP_CONFIG_BYTES` (32-bit and 64-bit pointers). The `_Static_assert`s
  in `sstep_ctrl.c` fail otherwise. Measure on x86-64 and Cortex-M4 and keep the
  exact values. The array size must stay a plain integer expression: no `sizeof`
  and no `offsetof` (Cppcheck 2.13 in CI reports `misra-config` / 10.4 for them).
- **Both backends.** Every arithmetic change must work with
  `SSTEP_MATH_BACKEND_FIXED` and `_FLOAT` with the same saturation ranges; the
  motion planner and the configuration code stay integer in both.
- **Configuration in integer engineering units** (`_mohm`, `_uh`, `_ma`, `_mv`,
  `_hz`); `sstep_init()` derives every gain without floating point
  (`2π = 710 / 113`) and validates every field (`SSTEP_ERR_PARAM` per field,
  `SSTEP_ERR_CONFIG` for combinations).
- `pwm_freq_hz` and `dead_time_ns` are reserved: present in the API, not used by
  the code. Do not document them as active.
- Public symbols use the `sstep_` / `SSTEP_` prefix; public headers keep the
  `extern "C"` guards and a Doxygen comment for every item; parameter names in
  declarations and definitions match (Rule 8.3).
- MISRA C:2012: `misra-analyze` reports 0 violations; a necessary violation gets an
  entry in the deviation register in `docs/misra.md` and an inline
  `cppcheck-suppress` comment. Single `return` per function, parenthesized mixed
  operators, `(void)` on ignored results, `++`/`--` as separate statements, array
  indexing instead of pointer arithmetic, no `_Alignof`/`_Generic` (Rule 1.4).
- Every public function validates its pointers (Dir 4.11);
  `SSTEP_CFG_ARG_CHECKS = 0` removes only the checks of `sstep_fast_loop()`.
- No warnings with `-std=c11 -Wall -Wextra -Wpedantic -Wconversion`, for the host
  compiler and `arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb`.
- Style: `.clang-format` (4 spaces, Allman braces, `extern "C" {` on one line, no
  space after casts, 100 columns); file banners and section comments as in the
  existing files (`.vscode/cpp.json.code-snippets`). Every tracked C/C++ file must be
  unchanged by `clang-format` 18 (the CI version): `format-check` fails otherwise.
  Generated code (`src/sstep_sin_table.c`) is wrapped in `/* clang-format off */`
  by its generator; one-line test tables use `// clang-format off`.
- Keep the public API stable within major version 1; record every user-visible
  change under `[Unreleased]` in `CHANGELOG.md`.

## Tests (definition of done)

```sh
cmake --preset "[Debug]Stepper Motor Control" && cmake --build "build/[Debug]Stepper Motor Control"
cmake --preset "[UnitTests]Stepper Motor Control" && cmake --build "build/[UnitTests]Stepper Motor Control" --target coverage
cmake --preset "[UnitTestsFloat]Stepper Motor Control" && cmake --build "build/[UnitTestsFloat]Stepper Motor Control" --target coverage
cmake --build "build/[Debug]Stepper Motor Control" --target misra-analyze
python tools/docs/check_snippets.py
python tools/sstep_gen_sin_table.py --check
```

- `coverage` fails below 100 % line or branch coverage of `src/`, in each backend.
  Delete stale `*.gcda` after source changes if gcov complains about checksums.
- Register new test files in `sstep.cmake`.
- Behavior changes of the control get a closed-loop test in
  `tests/unit/gtest_sstep_loop.cpp` (motor model in `sstep_test_plant.hpp`). The
  tests print `[ measured ]` lines; the documentation quotes them, so re-run
  `SSTEP_gtest --gtest_filter='SstepLoop.*'` and update the numbers in
  `docs/motor-control.md`, `docs/overview.md`, `README.md` and `mkdocs.yml`
  (`extra.landing`) when they change.
- Closed-loop results that must keep passing: 2 A current step with < 5 %
  overshoot and < 1 ms settling (measured 1.11 %, 0.40 ms), 6400-microstep move
  ending on target, load step below/above `Kt·I`, reversal, 8 rev/s topology limit
  (1.862 A full bridge, 1.557 A shared leg), bus voltage compensation, voltage drive.
- Stall detection (`SstepLoop.Stall*`, `SstepStall.*`): the hold-current slip
  (0.12 N·m, 1.5 A / 0.5 A) trips 48 ms after the load angle passes 90° and a hard stop
  after 50 ms; no fault in the robustness matrix on the nominal and the worst-case plant
  (largest detector level 32 %).

## Measured figures in the documentation

Every number in `docs/`, `README.md` and `mkdocs.yml` is measured; re-measure after
a change that can affect it.

| Figure | How |
| --- | --- |
| RAM per instance | `sizeof(struct sstep)` on Cortex-M4: 432 B fixed, 456 B float (64-bit host: 456 B, 480 B) |
| Flash | `arm-none-eabi-size` of `src/*.c` at `-Os`, Cortex-M4 (float with `-mfpu=fpv4-sp-d16 -mfloat-abi=hard`) |
| Stack | `-fstack-usage -fcallgraph-info=su`, then `python tools/docs/stack_usage.py <dir> --markdown`; table in `docs/configuration.md` (Cortex-M4, `-Os`/`-O0`, both backends) |
| Sine accuracy | Sweep `sstep_sin_cos()` over a full turn against `sin`/`cos` for table bits 6, 8, 10 |
| Closed loop | `[ measured ]` lines of the `SstepLoop` tests |

## Documentation

MkDocs Material website (`mkdocs.yml`, pages in `docs/`) and a PDF manual built from
the same pages (`python tools/docs/build_manual.py`). All documentation is in
English and written for embedded engineers integrating the library.

- Chapters: `overview`, `guide`, `architecture`, `usage`, `extending`,
  `configuration`, `motor-control` (algorithms and equations), `integration`,
  `api`, `misra`, `changelog`, `about`, `LICENSE`. There is no protocol, GUI or
  examples chapter.
- Formulas: LaTeX with `pymdownx.arithmatex` (`$…$` inline, `$$…$$` blocks); KaTeX
  renders them on the website (`docs/assets/javascripts/katex.js`), `ziamath`
  renders them as SVG in the PDF. Keep code identifiers in backticks, outside the
  math. Avoid `|` inside math in tables; use `\lvert … \rvert`.
- Diagrams and the animated hero: edit and run `python tools/docs/diagrams.py`,
  preview the SVGs, never edit them by hand.
- C snippets of `docs/*.md` compile with `tools/docs/check_snippets.py` against
  `tools/docs/snippet_stubs.h` (board functions) and
  `tools/docs/snippet_include/` (FreeRTOS stubs); API prototypes must match the
  headers exactly. Add a board stub when a snippet needs a new function.
- `mkdocs build` must print no WARNING.
- The documentation tooling is the shared base of the `sfs-protocol-library` skill.
  Keep `overrides/`, `svgkit.py`, `check_snippets.py`, `stack_usage.py` and the
  stylesheet as in its templates and change project data only (`mkdocs.yml`,
  `diagrams.py`, `snippets.json`). `build_manual.py` and `manual.css` differ from
  the templates by the formula rendering (arithmatex + ziamath); keep that part
  when syncing them.
- Brand: SystemFromScratch® exactly like that; do not name the trademark owner;
  STM32 is a trademark of STMicroelectronics.

## Releases

1. Set `SSTEP_DEFAULT_VERSION` in `cmake/version.cmake` and the fallbacks in
   `sstep.h` to the new version.
2. Move the `[Unreleased]` entries in `CHANGELOG.md` to `[X.Y.Z] - date`.
3. Merge to `main`, then push the tag `vX.Y.Z`. The Release workflow refuses a tag
   that does not match these files, runs the coverage and MISRA gates and publishes
   the library, headers, MISRA report and PDF manual as a GitHub Release.

Never commit, push a tag or publish a release without the maintainer's explicit
request.
