# Stepper-C

Hardware-agnostic stepper motor control library in C11 for embedded systems.

> **Status:** control loops implemented and tested on a motor model (both numeric
> backends, both power stage topologies). Next: software-in-the-loop with the Scilab
> model, then the B-G431B-ESC1 port.

## Overview

Stepper-C computes phase voltage commands for a two-phase hybrid stepper motor from a
motion command and the measured phase currents. Everything the library needs from the
hardware (PWM, ADC, time, faults) is passed in as callbacks at initialisation, so the
same code runs on any MCU.

- **Configuration-driven:** motor (R, L, Kt, steps/rev, rated current), supply voltage and
  power stage topology are given at initialisation; controller gains are derived from them.
- **Two power stage topologies:** two full H-bridges (4 legs) or three half-bridges with a
  shared leg (3 legs, for example STMicroelectronics B-G431B-ESC1).
- **Two numeric backends:** fixed point (Q15/Q31) for MCUs without an FPU and
  single-precision float for MCUs with an FPU, selected at build time.
- **Embedded rules:** C11, no heap, no RTOS, no global mutable state, MISRA C:2012.

## Related repositories

| Repository | Role |
| --- | --- |
| `sfs-stepper-ctrl` | this library |
| `sfs-stepper-sim` | Scilab/Xcos motor model and software-in-the-loop tests of this library |
| `sfs-stepper-power-spice` | ngspice models of the power stage |

## Building

```bash
cmake --preset "[Debug]Stepper Motor Control"
cmake --build "build/[Debug]Stepper Motor Control"
```

## License

Apache License 2.0, see [LICENSE](LICENSE). SystemFromScratch® is a registered trademark
of Anton Vynohradov.
