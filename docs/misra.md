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
| Cppcheck MISRA addon | TODO violations (versions) |
| Unit test coverage of `src/` | TODO |
| Deviations | TODO |

## Manual review notes

| Guideline | How the library complies |
| --- | --- |
| Dir 4.1 (run-time failures) | TODO |
| Dir 4.11 (validity of arguments) | TODO |
| Dir 4.12, Rule 21.3 (dynamic memory) | No heap. |
| Rule 15.5 (single point of exit) | One `return` per function. |
| Rule 17.2 (recursion) | No recursion. |
| Rule 21.6 (standard I/O) | No `<stdio.h>`. |

## Deviation register

### DEV-01: TODO (Rule/Dir, category)

- **Location:** TODO
- **Rationale:** TODO
- **Risk control:** TODO
