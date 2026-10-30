# How It Works

TODO one paragraph: what this chapter explains.

## Layers

![Stepper-C architecture](assets/images/architecture.svg)

| Layer | Header | Responsibility |
| --- | --- | --- |
| Definitions | `sstep_defs.h` | TODO |
| Codec | `sstep_protocol.h` | TODO |
| Link | `sstep_link.h` | TODO |

## Data flow

![Receive and reply data flow](assets/images/data-flow.svg)

**Receive path.** TODO

**Transmit path.** TODO

## The parser

![Parser algorithm](assets/images/parser-flow.svg)

1. **Append.** TODO
2. **Synchronize.** TODO
3. **Check the length.** TODO
4. **Wait.** TODO
5. **Validate and decode.** TODO
6. **Reset.** TODO

!!! note "Recovery after errors"
    TODO what is lost after a corrupted length, and what the controller should do.

## The encoder

TODO escaping, capacity checks, checksum.

## Memory model

| Structure | Contents | Size with the default configuration |
| --- | --- | --- |
| TODO | TODO | TODO |

## Error handling

| Situation | Result |
| --- | --- |
| A `NULL` pointer or inconsistent length | `SSTEP_PARSE_ERROR`; nothing is written |
| TODO | TODO |

## Threading and reentrancy

- **Codec functions** are reentrant.
- **A link** is not thread-safe; call receive from one context, never from an ISR.
- **Sending** from several contexts needs the application's mutex.
- **The handler** data is valid only during the call.

## Design decisions

| Decision | Reason |
| --- | --- |
| One byte per parser call | Works with any transport chunking. |
| TODO | TODO |
