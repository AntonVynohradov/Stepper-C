# Introduction

Stepper-C is a C11 library that drives a two-phase hybrid stepper motor with closed-loop
sinusoidal current control. Given the measured phase currents and bus voltage, it computes
the PWM compare values of the power stage, and it turns motion commands (move to a
position, run at a velocity, stop) into a smooth microstep trajectory. It runs on any MCU:
the hardware is reached only through callbacks, and the numeric backend is either fixed
point (no FPU needed) or single-precision float.

## What the library does

- **Controls the phase currents.** One PI controller per phase with back-EMF
  feed-forward and anti-windup, gains derived from the motor's R and L and the requested
  bandwidth. An open-loop voltage mode is available for boards without current sensing.
- **Generates the microstep angle.** Sine/cosine current references from a 32-bit
  electrical angle, 1 to 256 microsteps per full step.
- **Plans the motion.** Trapezoidal profile with separate acceleration and deceleration,
  no overshoot, smooth replacement of a running command; constant-velocity mode; stop.
- **Fits the voltage to the power stage.** Two full H-bridges (4 legs) or three
  half-bridges with a shared leg (3 legs); the request is limited to what the topology and
  the measured bus voltage can deliver, keeping its direction.
- **Protects the hardware.** Overcurrent (both phases and the shared leg), overvoltage and
  undervoltage trips in the fast loop, a bus voltage check before the outputs go on, a
  self-test of the power stage and the motor wiring, hardware-callback errors and external
  faults; outputs off within the same period ([Protection](protection.md)).
- **Detects a stall without a sensor.** A back-EMF observer reports a rotor that stops,
  falls out of synchronism or slips at standstill (`SSTEP_FAULT_STALL`, optional).
- **Reduces the current at standstill.** Hold current after a configurable delay.

## What the library does not do

| Concern | Why it is not in the library |
| --- | --- |
| Timer, PWM and ADC drivers | Hardware-specific; you start the conversions and pass raw ADC counts in, and write the compare values out in a callback. |
| Interrupt and task setup | Every system has its own; the library only defines which function runs at which rate. See the [integration guide](integration.md). |
| Position feedback (encoder) | The library is an open-loop positioner with closed-loop current; the rotor position is not measured. Sensorless stall detection reports a lost synchronism but does not correct the position. |
| Homing, limit switches, G-code, multi-axis coordination | Application logic on top of `sstep_move_to()`, `sstep_set_position()` and the status. |
| Motor parameter identification | R, L and Ke come from the datasheet or a measurement. |
| Communication (UART, CAN, Modbus…) | Out of scope; call the API from your protocol handler. |

## Who it is for

- **Firmware for stepper drives** on a general-purpose MCU with gate drivers, for example
  STMicroelectronics B-G431B-ESC1 or a custom board with two H-bridges.
- **Machines with several axes,** one driver instance per motor, no shared state.
- **Projects headed for review or certification:** MISRA C:2012, 100 % coverage, no heap,
  bounded execution time.

## Key properties

| Property | Value |
| --- | --- |
| Language | C11, no warnings with `-Wall -Wextra -Wpedantic -Wconversion` |
| Dependencies | `<stddef.h>`, `<stdint.h>`, `<stdbool.h>` only |
| Numeric backends | Fixed point Q15/Q31 (integer only) or single-precision float, chosen at build time |
| Memory | No heap, no global or static mutable data (`.data` and `.bss` are empty) |
| RAM per instance | 496 B fixed point, 528 B float on Cortex-M4 ([details](configuration.md#memory-footprint)) |
| Flash | 10.8 KiB fixed point, 11.5 KiB float (Cortex-M4, `-Os`, both topologies) |
| Stack | `sstep_fast_loop()` 216 B, `sstep_slow_loop()` 152 B worst case (Cortex-M4 `-Os`, [details](configuration.md#stack-usage)) |
| Coding standard | MISRA C:2012, 0 Cppcheck addon violations, [1 deviation](misra.md) |
| Tests | 100 % line and branch coverage of `src/` in every build variant; closed-loop tests on a motor model |
| Thread safety | One fast-loop context and one slow-loop context per instance, by design; see [Threading](architecture.md#threading-and-reentrancy) |
| License | [Apache License 2.0](LICENSE.md) |

## Terminology

Full step
:   1/200 of a revolution for a 1.8° motor; a quarter of an electrical period.

Microstep
:   A full step divided by `microsteps` (power of two, 1 to 256). Positions and
    velocities of the API are in microsteps.

Electrical angle
:   Angle of the current vector in the two phases; one electrical period is four full
    steps. Held as a 32-bit unsigned number, one turn = $2^{32}$.

Per unit (pu)
:   Internal signals are relative to a base value: currents to `i_full_scale_ma`,
    voltages to `vbus_nominal_mv`. 1.0 pu is the base.

Fast loop
:   `sstep_fast_loop()`, called from the PWM/ADC interrupt at `fast_loop_hz`
    (typically 20 kHz): current control and PWM.

Slow loop
:   `sstep_slow_loop()`, called at `slow_loop_hz` (typically 1 kHz): trajectory,
    angle, currents, events.

Leg
:   One half-bridge (high-side and low-side switch) of the power stage, driven by one
    PWM compare value.

Load angle
:   Electrical angle between the commanded current vector and the rotor; it grows with
    the load torque and the motor loses steps when it exceeds 90° (180° to the next
    stable position).

## Conventions in this documentation

- Engineering units in the API are integers with the unit in the field name:
  `_mohm`, `_uh`, `_ma`, `_mv`, `_hz`, `_ms`, `_ns`.
- "Fixed point" and "float" name the two numeric backends; numbers are given for both
  when they differ.
- The example motor is a 17HS19-2004S1 (1.6 Ω, 3.0 mH, 2.0 A, 0.21 N·m/A, 200 steps)
  on a 24 V supply; the example board is a B-G431B-ESC1 (three half-bridges).
