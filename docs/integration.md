# Integration Guide

The library does not touch hardware. An integration has three parts:

1. **Receive:** move bytes from the transport into `sstep_link_receive()` from a thread
   or the main loop, not from an interrupt handler.
2. **Transmit:** a write function that sends a complete frame; serialize it if several
   threads send.
3. **Application:** a handler that interprets frames and answers.

Complete, buildable programs for each pattern are described in [Examples](examples.md).

## RTOS: interrupt, queue and RX thread

## Adding the library to a CMake build

```cmake
include(path/to/Stepper-C/sstep.cmake)
target_link_libraries(app PRIVATE sstep)
target_link_libraries(sstep PUBLIC zephyr_interface)   # Zephyr applications only
```

## Bare metal and host

## Notes from hardware bring-up
