# Configuration

Build-time parameters live in `inc/sstep_config.h`. Each one is guarded by `#ifndef`
and range-checked at compile time; override it with a compiler definition.

| Parameter | Default | Meaning |
| --- | --- | --- |
| `SSTEP_MAX_…` | TODO | TODO (range checked at compile time) |

## Memory footprint

| Object | Size |
| --- | --- |
| TODO | TODO |

## Stack usage

Measured with `-fstack-usage`, default configuration, bytes per function frame:

| Function | Cortex-M4 `-Os` | Cortex-M4 `-O0` | x86-64 `-O2` |
| --- | --- | --- | --- |
| TODO | TODO | TODO | TODO |

Toolchains: TODO versions.

| Path | Chain | Stack |
| --- | --- | --- |
| Transmit | TODO | TODO B + write callback |
| Receive | TODO | TODO B + handler |

```bash
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -std=c11 -fstack-usage -Iinc -I. -c src/*.c
cat *.su
```

## CMake options

| Option | Default | Meaning |
| --- | --- | --- |
| `SSTEP_UNIT_TESTS` | `OFF` | GoogleTest suite and `coverage` target |
| `SSTEP_INTEGRATION_TESTS` | `OFF` | pytest suite |
| `SSTEP_EXAMPLES` | `OFF` | Host examples and the bare-metal simulation test |
| `SSTEP_COVERAGE_MIN` | `100` | Coverage gate |
| `SSTEP_MISRA_FAIL_ON_VIOLATION` | `ON` | MISRA gate |
| `SSTEP_MISRA_RULE_TEXTS` | empty | Licensed MISRA rule texts |
