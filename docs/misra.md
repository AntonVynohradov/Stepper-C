# MISRA C:2012 compliance

This page records how the library is checked against MISRA C:2012 and every deviation from
it. It covers the library code only: `src/`, `inc/` and `sstep.h`.

## Checking

| Check | Where | Gate |
| --- | --- | --- |
| Cppcheck MISRA C:2012 addon | `misra-analyze`, CI *MISRA C:2012 Analysis* | Fails on any violation |
| Cppcheck, all checks | `cppcheck-analyze`, CI *Cppcheck Analysis* | Report |
| CodeQL `cpp-queries` | `codeql-analyze`, CI *CodeQL Analysis* | Report |
| Compiler | `-std=c11 -Wall -Wextra -Wpedantic -Wconversion` | No warnings |
| Manual review | Pull request review | Reviewer |

## Current status

| Item | Result |
| --- | --- |
| Cppcheck MISRA addon | 0 violations (Cppcheck 2.16.2) |
| Unit test coverage of `src/` | 100 % lines, branches and functions, fixed point and float (gcovr 8.6) |
| Deviations | 1 (DEV-01) |

## Manual review notes

| Guideline | How the library complies |
| --- | --- |
| Dir 4.1 (run-time failures) | Every arithmetic operation of the numeric backend saturates; shifts of signed values use `sstep_asr()`; divisions are by validated non-zero values; the gains are range-checked by `sstep_init()`. |
| Dir 4.11 (validity of arguments) | Every public function checks its pointers and ranges and returns `SSTEP_ERR_PARAM`; `SSTEP_CFG_ARG_CHECKS = 0` removes only the `NULL` checks of `sstep_fast_loop()`. |
| Dir 4.12, Rule 21.3 (dynamic memory) | No heap. |
| Rule 15.5 (single point of exit) | One `return` per function. |
| Rule 17.2 (recursion) | No recursion. |
| Rule 21.6 (standard I/O) | No `<stdio.h>`. |

## Deviation register

### DEV-01: Rule 11.3 (required), opaque driver instance

- **Location:** `sstep_ctx_of()` and `sstep_ctx_of_const()` in `src/sstep_internal.h`, the
  only casts from `struct sstep *` to `struct sstep_ctx *`. Marked with
  `cppcheck-suppress misra-c2012-11.3`.
- **Rationale:** The application allocates the driver instance (no heap, no global state),
  so it must know its size, but not its layout. The public `struct sstep` in `sstep_ctrl.h`
  is an array of `uint64_t` of that size; the private `struct sstep_ctx` holds the state.
  This keeps the internal types (gains, integrators, setpoints, numeric backend) out of
  the application code and lets them change without breaking the API.
- **Risk control:** `sstep_ctrl.c` checks at compile time that `struct sstep` is at least
  as large and as aligned as `struct sstep_ctx`; a layout change that outgrows
  `SSTEP_STATE_BYTES` does not compile. The application never accesses
  `struct sstep::opaque`, so the storage is only ever accessed as `struct sstep_ctx`.
  The cast is applied to every public entry point before any member access, and the NULL
  check is done on its result (a NULL pointer converts to NULL).
