# API Reference

This chapter documents every public function, type and macro of Stepper-C. Include the
umbrella header:

```c
#include "sstep.h"
```

| Header | Contents |
| --- | --- |
| `sstep.h` | All headers; library version |
| `sstep_config.h` | Build-time configuration |
| `sstep_defs.h` | Protocol constants, byte-order helpers |
| `sstep_protocol.h` | Checksum, stream parser, encoder |
| `sstep_link.h` | Transport-independent link |

## Version — `sstep.h`

## Configuration — `sstep_config.h`

## Definitions — `sstep_defs.h`

## Codec — `sstep_protocol.h`

TODO for every function: prototype in a ```` ```c ```` block, one paragraph, a
Parameter | Description table and a Return | Meaning table.

## Link — `sstep_link.h`

## Return codes summary

| Value | Meaning |
| --- | --- |
| `0` | Success |
| `SSTEP_PARSE_ERROR` (`-1`) | Invalid argument, malformed frame or insufficient capacity |
