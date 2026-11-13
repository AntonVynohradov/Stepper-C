# Changelog

All notable changes to this project are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.4.0] - 2026-11-13

### Added

- **Build-time option `SSTEP_CFG_STALL`** (default 1), like `SSTEP_CFG_SELFTEST`: 0 removes
  the stall detection (`sstep_stall.c`, the observer in the fast loop, the decision in the
  slow loop and their state) under `#if`, without constant run-time branches.
  `sstep_init()` returns `SSTEP_ERR_UNSUPPORTED` for `stall_time_ms > 0`, the estimates of
  the status (`emf_d_mv`, `emf_q_mv`, `rotor_velocity`, `load_angle_cdeg`,
  `stall_level_pct`) are 0 and an overcurrent no longer carries `SSTEP_FAULT_STALL`.
  Saves 2160 B of flash (2400 B float) and 80 B per instance on Cortex-M4
  (`SSTEP_STALL_BYTES`). Without self-test and stall detection the library takes 7504 B of
  flash (8054 B float) and 368 B (400 B) per motor.
- New unit test preset `SSTEP[UnitTestsNoStall]` (`tests/config/no_stall`), run by CI and
  the release workflow like the other `[UnitTests` presets; `SstepStall.CompiledOut`.
- `docs/configuration.md`: footprint of every combination of the two options and of the
  topologies, and the linked size with `--gc-sections` and `-flto`.

### Fixed

- **Self-test: false results with a moving rotor.** The energised phase pulls the rotor by
  up to a full step, and a rotor swinging into it induces currents in both phases. The
  checks judged one sample, so a rotor still moving failed a healthy motor: on a
  B-G431B-ESC1 with the phases swapped and the rotor a full step away at 24 V,
  `SSTEP_SELFTEST_WRONG_PHASE`; on the motor model with a loaded rotor (7 to 20 times the
  inertia), `SSTEP_SELFTEST_SHORT` and `SSTEP_SELFTEST_WRONG_PHASE` in phase B. Now the hold
  ends when the other phase has stayed below half the own current for 2 ms in a row, the
  decay when both currents have stayed below a quarter of the test current for 5 ms in a
  row, and twice the test current is a short only without a break for 100 ms; each fails
  after 100 ms (`WRONG_PHASE`, `LEAK`, `SHORT`). The overcurrent trip still acts at once.

### Changed

- The self-test takes 42 ms on the motor model with a free rotor (was 39 ms), 39 to 55 ms
  with a loaded rotor a full step away, at most about 440 ms with a rotor that keeps moving.
- `SSTEP_SELFTEST_BYTES` 48 B (was 40 B): `struct sstep` on Cortex-M4 496 B fixed point,
  528 B float (64-bit hosts 544 B, 576 B). Flash +216 B for the self-test.
- Float backend: `sstep_gain_from_ratio64()` converts its `int64_t` operands to `float`
  from two 32-bit halves (`float_of_i64()`), which the FPU converts in hardware. The
  conversion of an `int64_t` (`__aeabi_l2f`) linked the software float addition of libgcc:
  an image with the whole API is 484 B smaller on Cortex-M4. The results of the
  closed-loop tests are unchanged.
- CI and release run on `${{ vars.CI_RUNNER || 'ubuntu-24.04' }}` instead of
  `ubuntu-latest`, changed in one place with the repository variable `CI_RUNNER`.
  `ubuntu-latest` moved to Ubuntu 26.04 with Cppcheck 2.19, whose MISRA addon reports
  Rule 7.3 (lowercase `l` suffix) on the identifiers `accel` and `decel` and failed the
  release gate; the MISRA gate stays on Cppcheck 2.13.
- `SSTEP_STATE_BYTES` is the sum of a base and `SSTEP_STALL_BYTES` and
  `SSTEP_SELFTEST_BYTES`.
- The stall detection takes 80 B per instance on Cortex-M4; the documentation said 96 B.
- Flash on Cortex-M4 at `-Os`: 11032 B fixed point, 11814 B float (library objects, without
  the 64-bit division of libgcc). Stack of `sstep_init()` re-measured: 188 B float `-Os`,
  1608 B / 1552 B at `-O0`.

## [1.3.0] - 2026-11-12

### Added

- **Sampling window** (`sample_window_ticks` in the stage configuration): every leg compare
  value stays at or below `pwm_period_ticks − sample_window_ticks`, so low-side shunts can
  be sampled in every period. The voltage limit shrinks by the same fraction, with the
  direction of the voltage vector kept and the anti-windup and stall detector on the applied
  voltage. Found on the B-G431B-ESC1: at the voltage limit of a 1.5 A current step the
  duty cycle reached 100 %, the bus voltage sample was corrupted and the undervoltage trip
  fired. 0 keeps the previous behaviour.
- **Current slew** (`i_slew_ma_per_ms` in the control configuration): the current
  amplitude rises from 0 after `sstep_enable()` and moves between run and hold current and
  after `sstep_set_current()` at a limited rate. 0 keeps the previous step.
- **Self-test** `sstep_selftest()`: before the first enable the fast loop drives each phase
  in turn up to a test current at a slowly rising voltage and finds a conducting switch, an
  open phase, a short, a current of the wrong sign and a phase on the wrong legs. Result in
  `sstep_status` (`selftest`, `selftest_phase`, `selftest_i_ma`, `selftest_duty`), end
  reported with `SSTEP_EVENT_SELFTEST_DONE`, failure latched as `SSTEP_FAULT_SELFTEST`. Both
  topologies; uses `dead_time_ns` and `pwm_freq_hz` for the duty levels.
- **Shared-leg overcurrent trip:** in `SSTEP_TOPOLOGY_SHARED_LEG` the current of the common
  leg, $\lvert i_A + i_B \rvert$, trips at `i_trip_ma` like the phases.
- `i_trip_voltage_drive` in the control configuration: overcurrent trips also in
  `SSTEP_DRIVE_VOLTAGE` for boards that measure the currents there.
- `SSTEP_ERR_VBUS`: `sstep_enable()` and `sstep_selftest()` refuse a bus voltage outside the
  trip limits without latching a fault.
- **Build-time option `SSTEP_CFG_SELFTEST`** (default 1), like the topology options:
  0 removes the self-test code and its state; `sstep_selftest()` then returns
  `SSTEP_ERR_UNSUPPORTED`. Saves 1144 B of flash (fixed point; 1140 B float) and 40 B per
  instance on Cortex-M4. New unit test preset `SSTEP[UnitTestsNoSelftest]`, 100 % coverage.
- Protection chapter in the documentation: every protection of the library and what
  the application adds.

### Changed

- `sstep_enable()` with the last bus voltage sample outside the trip limits returns
  `SSTEP_ERR_VBUS` and stays disabled; before, it enabled and the first fast loop period
  latched the undervoltage or overvoltage fault.
- In `SSTEP_TOPOLOGY_SHARED_LEG` a current of the common leg above `i_trip_ma` now trips
  (see Added): with phase currents near the trip the combined current can trip first.
- `struct sstep` on Cortex-M4: 488 B fixed point, 520 B float (was 432 B, 456 B), of which
  40 B self-test (`SSTEP_SELFTEST_BYTES`); 448 B / 480 B with `SSTEP_CFG_SELFTEST = 0`.
  `SSTEP_CONFIG_BYTES` 156 B (192 B on 64-bit hosts), `SSTEP_STATE_BYTES` 332 B / 364 B
  (344 B / 376 B on 64-bit hosts). Flash 10816 B / 11542 B at `-Os`;
  9672 B / 10402 B without the self-test.

## [1.2.0] - 2026-11-06

### Fixed

- **Stall detection missed a slipping rotor at low speed.** The back-EMF vector of a slipping
  rotor turns in the commanded frame and stayed above `stall_sync_pct` about half of the
  time, so the up/down counter did not accumulate: a heavy axis (load inertia ×100) lost
  hundreds of full steps without a fault. A reversed back-EMF now keeps the condition true
  for half of the trip time: `SSTEP_FAULT_STALL` 43 to 60 ms after the load angle passes 90°.
- **False stall under a heavy load.** The inductance error adds to $\cos\delta$ (it does not
  scale it, as the documentation said), so a synchronous rotor at 72 % of the pull-out torque
  with $R$ +30 %, $L$ −20 % tripped. The condition in motion now also requires the d axis
  inside the window of a rotor that does not turn. The same window removes the false trip at
  the voltage limit: 11 rev/s at 24 V now runs with the example `stall_sync_pct` of 50.

### Changed

- `SSTEP_FAULT_OVERCURRENT` comes **together with `SSTEP_FAULT_STALL`** when the trip finds
  the stall detector counting: a light rotor reaches the trip before `stall_time_ms`, and the
  position is lost. An application that compares `faults` with a single flag must test the
  bits. Without stall detection nothing changes.
- A load angle swinging repeatedly beyond about 120° near the voltage limit is reported as a
  stall even before a step is lost (documented in Limits).
- `SSTEP_STATE_BYTES` on 64-bit hosts: 288 B fixed point, 312 B float (Cortex-M4 unchanged:
  280 B, 304 B). Flash on Cortex-M4 (`-Os`): 9216 B fixed point, 9894 B float. Worst-case
  stack of `sstep_slow_loop()` 152 B fixed point (float 140 B); the slow loop executes about
  145 more instructions per period with stall detection on (`-Os`); the fast loop is unchanged.
- The version comes only from the `vX.Y.Z` Git tag. `sstep.cmake` now applies it to the
  applications that include it, and the headers of the release package carry it. A header
  taken from the repository and built without CMake reports `0.0.0` (unknown). `CITATION.cff`
  no longer states a version or a release date.

## [1.1.0] - 2026-11-01

Sensorless stall detection. Compatible with 1.0.0: the new configuration fields are
zero in existing initializers, which keeps stall detection off and the behaviour
unchanged; rebuild the application with the new headers (the instance size changed).

### Added

- **Stall detection** (`src/sstep_stall.c`): a back-EMF observer in the fast loop
  (e = v − R·i − L·di/dt from the applied voltage, the measured currents and the
  commanded angle, filtered in the commanded current frame) and a decision in the slow
  loop. In motion it checks that the back-EMF along the commanded motion stays above
  `stall_sync_pct` of Ke·ω; at rest and at low speed it checks that the rotor does
  not turn faster than the command by more than `stall_speed`, with the observer offset
  (resistance, dead time) learned while the rotor follows. An up/down counter raises the
  new fault `SSTEP_FAULT_STALL` (`0x0020U`) after `stall_time_ms`, with the usual fault
  behaviour (outputs off, `SSTEP_STATE_FAULT`, `on_fault()`, cleared by
  `sstep_clear_faults()`).
- **Configuration** (`struct sstep_ctrl_cfg`): `stall_time_ms` (0 = off, the default),
  `stall_filter_us`, `stall_speed`, `stall_sync_pct`. `sstep_init()` returns
  `SSTEP_ERR_CONFIG` for stall detection with voltage drive or without
  `ke_uv_s_per_rad`.
- **Status** (`struct sstep_status`): `emf_d_mv`, `emf_q_mv`, `rotor_velocity`,
  `load_angle_cdeg`, `stall_level_pct`.
- **Documentation:** the stall detection section of Motor Control (principle, the exact
  equations for a model, design decisions, parameter choice, measured results, cost,
  limits), configuration, usage, integration and API updates. New hero illustrations
  (`docs/assets/images/hero-dark.png` for the website and the PDF cover,
  `hero-light.png` for the README).

### Changed

- `struct sstep` grows by 96 B (432 B fixed point, 456 B float on Cortex-M4):
  `SSTEP_CONFIG_BYTES` 152 / 176 B, `SSTEP_STATE_BYTES` 280 / 304 B.
  `struct sstep_status` grows from 32 B to 48 B.
- Flash on Cortex-M4 (`-Os`): 9040 B fixed point, 9718 B float. Worst-case stack of
  `sstep_fast_loop()` 216 B fixed point (128 B float), of `sstep_get_status()` 92 B.
- With stall detection on, the fast loop executes about 240 more instructions per
  period (`-O2`, both backends; 475 fixed point and 213 float with `-Os`).

### Quality

- 93 GoogleTest tests (fixed point) and 92 (float), 100 % line, branch and function
  coverage of `src/` in both backends. The motor model of the tests gained a detent
  torque, a dead-time voltage error, a configurable current quantization and an imposed
  rotor motion for synthetic tests.
- Stall detection on the motor model: the reference case (17HS19-2004S1, three-leg stage,
  24 V, 0.12 N·m held at 1.5 A but not at the 0.5 A hold current) reports
  `SSTEP_FAULT_STALL` 48 ms after the load angle passes 90°; a hard stop at 3 rev/s after
  50 ms. No fault through enable, run → hold, moves, reversal and runs from 0.125 to
  11 rev/s, also with R +30 %, L −20 %, 70 mV dead time per leg, 29 mA current
  quantization and detent torque (largest detector level 32 %).
- MISRA C:2012: 0 violations, no new deviation.

## [1.0.0] - 2026-10-31

First release: closed-loop microstepping current control for two-phase hybrid stepper
motors in portable C11.

### Added

- **Public API** (`sstep.h`, 15 functions): `sstep_init()`, `sstep_enable()`,
  `sstep_disable()`, `sstep_fast_loop()`, `sstep_slow_loop()`, `sstep_move_to()`,
  `sstep_move_by()`, `sstep_run()`, `sstep_stop()`, `sstep_set_position()`,
  `sstep_set_current()`, `sstep_set_motion_limits()`, `sstep_report_fault()`,
  `sstep_clear_faults()`, `sstep_get_status()`.
- **Types** (`inc/sstep_ctrl.h`, `inc/sstep_defs.h`): configuration in integer
  engineering units (motor, power stage and measurement, control and motion), hardware
  callbacks given at initialization (`pwm_write`, `output_enable`, optional `on_fault`,
  `on_event`), raw ADC sample, status in engineering units, status codes, fault flags,
  driver states. `pwm_freq_hz` and `dead_time_ns` are reserved for later versions.
- **Opaque driver instance:** `struct sstep` is `uint64_t opaque[SSTEP_INSTANCE_WORDS]`,
  allocated by the application (336 B fixed point, 360 B float on Cortex-M4); the
  driver state and every numeric type stay private in `src/`.
- **Current control:** PI controller per phase with gains derived from R, L and the
  bandwidth (`Kp = L wc`, `Ki = R wc`), back-EMF feed-forward, back-calculation
  anti-windup, bus voltage compensation; open-loop voltage drive for boards without
  current sensing.
- **Microstepping:** 1 to 256 microsteps per full step, 32-bit electrical angle,
  quarter-wave sine table with linear interpolation (generated by
  `tools/sstep_gen_sin_table.py`), angle interpolated at the fast-loop rate.
- **Motion planner:** trapezoidal moves without overshoot in integer arithmetic
  (positions in 1/65536 microstep, 64 bits), separate acceleration and deceleration,
  smooth retargeting, constant-velocity runs, stop, homing with `sstep_set_position()`,
  hold current after a standstill delay, `SSTEP_EVENT_MOVE_DONE` and
  `SSTEP_EVENT_HOLD_CURRENT`.
- **Power stages:** two H-bridges (4 legs) and three half-bridges with a shared leg
  (3 legs); the voltage request is limited to the topology with its direction kept.
- **Protection:** overcurrent, overvoltage and undervoltage trips in the fast loop,
  callback errors and external faults switch the outputs off in the same period.
  Every function except `sstep_init()` returns `SSTEP_ERR_STATE` for an instance
  that was not initialized successfully, without touching the hardware.
- **Numeric backends:** fixed point (Q15/Q31, integer only) and single-precision float,
  selected with `SSTEP_MATH_BACKEND` (CMake) or `SSTEP_CFG_MATH_BACKEND`.
- **Build-time options** (`inc/sstep_config.h`, overridable in `sstep_user_config.h`):
  topologies, largest microstep division, sine table size, argument checks of the fast
  loop.
- **Build:** CMake library target in `sstep.cmake`, presets for Debug, Release and the
  unit tests of both backends; `misra-analyze`, `cppcheck-analyze` and `codeql-analyze`
  targets.
- **Documentation:** website and PDF manual: introduction, getting started,
  architecture, usage, extending, configuration with the measured stack and memory
  footprint, motor control theory with measured results, integration guide, API
  reference, MISRA compliance.

### Quality

- 78 GoogleTest tests (fixed point) and 77 (float): numeric backend, power stage,
  motion planner, API and closed loop on a motor model (`tests/unit/sstep_test_plant.hpp`);
  100 % line, branch and function coverage of `src/` in both backends.
- Closed loop on the motor model (17HS19-2004S1, 24 V, 20 kHz, 1 kHz bandwidth), matching
  the Scilab prototype: current step overshoot 1.11 %, settling 0.40 ms; 8 rev/s current
  1.862 A with two H-bridges and 1.557 A with a shared leg; load angle at 0.25 N·m
  37.21°.
- MISRA C:2012: 0 violations (Cppcheck 2.13 and 2.16), one documented deviation
  (DEV-01, Rule 11.3, opaque instance).
- No warnings with `-std=c11 -Wall -Wextra -Wpedantic -Wconversion` (GCC on x86-64 and
  `arm-none-eabi-gcc` for Cortex-M4); clean under AddressSanitizer and
  UndefinedBehaviorSanitizer.
- Cortex-M4 (GCC 15.2, `-Os`): 6980 B flash fixed point, 7446 B float; worst-case stack
  of `sstep_fast_loop()` 184 B / 128 B; `.data` and `.bss` empty.
- Sine table error (default 8 bits): 1.0 LSB of Q15 (fixed point), 4.8·10^-6 (float).

[1.4.0]: https://github.com/AntonVynohradov/Stepper-C/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/AntonVynohradov/Stepper-C/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/AntonVynohradov/Stepper-C/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/AntonVynohradov/Stepper-C/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/AntonVynohradov/Stepper-C/releases/tag/v1.0.0
