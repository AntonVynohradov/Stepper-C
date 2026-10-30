# Introduction

TODO one paragraph: Stepper-C is a C11 library that implements the wire format of
[Stepper Motor Control](TODO-specification-url) for embedded systems that must be predictable and reviewable.

## What the library does

- **Encodes frames.** TODO
- **Parses a byte stream.** TODO (any chunking, validation, resynchronization)
- **Connects the codec to a transport.** TODO (link, handler, write function)
- **Defines the protocol constants.** TODO

## What the library does not do

| Concern | Why it is not in the library |
| --- | --- |
| Drivers (UART, CAN, USB…) | Hardware- and OS-specific; you pass bytes in and provide a write function. |
| Threads, queues, interrupts, locking | Every system has its own; see the [integration guide](integration.md). |
| Device models / application data | TODO |
| Timeouts and retries | The library has no clock. |

## Who it is for

- **Device firmware.** TODO
- **Controller firmware.** TODO
- **Host tools.** TODO

## Key properties

| Property | Value |
| --- | --- |
| Language | C11, no warnings with `-Wall -Wextra -Wpedantic -Wconversion` |
| Dependencies | `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`, `<string.h>` only |
| Memory | No heap; caller-owned structures; sizes fixed by `SSTEP_MAX_…` |
| RAM per instance | TODO measured |
| Stack | TODO measured ([details](configuration.md#stack-usage)) |
| Coding standard | MISRA C:2012, 0 Cppcheck addon violations, [TODO deviations](misra.md) |
| Tests | 100 % line and branch coverage; C/Python cross-check; specification reference frames |
| Thread safety | None built in, by design; see [Threading and reentrancy](architecture.md#threading-and-reentrancy) |
| License | [Apache License 2.0](LICENSE.md) |

## Terminology

TODO term
:   TODO definition (controller, device, frame types, broadcast, escaping…).

## Conventions in this documentation

- Hexadecimal bytes without prefix inside frames (`AA 55`), with `0x` in text and code.
- Multi-byte fields are TODO endian, as on the wire.
- `SSTEP_MAX_…` is shown with its default value.
