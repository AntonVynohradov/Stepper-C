# Extending the Library

TODO one paragraph: extension points; almost everything is done on top of the library.

## Where to extend

| You want to… | Do it in | Library change needed? |
| --- | --- | --- |
| Support another transport | your write function and receive loop | No |
| TODO | TODO | TODO |

## Adding a transport

## Building device models

## Building multi-device commands

## Vendor-specific values

## Changing the configuration

## Contributing to the library

1. **Code.** C11, no new dependencies, no heap, MISRA rules used in the code.
2. **Tests.** 100 % line and branch coverage; update the Python twin when the codec changes.
3. **Analysis.** `misra-analyze` 0 violations or a recorded deviation.
4. **Format.** `clang-format` reports no changes.
5. **Documentation.** Pages, stack table, diagrams, `CHANGELOG.md` `[Unreleased]`.
6. **Compatibility.** Public API stable within a major version.
