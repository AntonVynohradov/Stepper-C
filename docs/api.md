# API Reference

This chapter documents every public function, type and macro of Stepper-C. Include one
header; it declares every function and includes the public type headers:

```c
#include "sstep.h"
```

| Header | Contents |
| --- | --- |
| `sstep.h` | All public functions; library version; includes the headers below |
| `sstep_config.h` | Build-time configuration |
| `sstep_defs.h` | Status codes, topologies, fault flags, driver states |
| `sstep_ctrl.h` | Configuration, callbacks, measurements, status, driver instance |

Everything else (`src/`) is private and not installed with the library.

## Version — `sstep.h`

| Macro | Value | Meaning |
| --- | --- | --- |
| `SSTEP_VERSION_MAJOR`, `…_MINOR`, `…_PATCH` | `1`, `1`, `0` | Version numbers; set by CMake from the latest `vX.Y.Z` tag, these are the fallbacks |
| `SSTEP_VERSION` | `"1.1.0"` | Version string |

## Configuration — `sstep_config.h`

| Macro | Default | Meaning |
| --- | --- | --- |
| `SSTEP_MATH_BACKEND_FIXED`, `SSTEP_MATH_BACKEND_FLOAT` | `0`, `1` | Values of `SSTEP_CFG_MATH_BACKEND` |
| `SSTEP_CFG_MATH_BACKEND` | `SSTEP_MATH_BACKEND_FIXED` | Numeric backend |
| `SSTEP_CFG_TOPOLOGY_FULL_BRIDGE` | `1` | Compile the full-bridge topology |
| `SSTEP_CFG_TOPOLOGY_SHARED_LEG` | `1` | Compile the shared-leg topology |
| `SSTEP_CFG_MICROSTEPS_MAX` | `256U` | Largest microstep division |
| `SSTEP_CFG_SINE_TABLE_BITS` | `8U` | Size of the sine table |
| `SSTEP_CFG_ARG_CHECKS` | `1` | `NULL` checks in `sstep_fast_loop()` |

Ranges and override rules: [Configuration](configuration.md#build-time-options).

## Definitions — `sstep_defs.h`

### Status codes

| Macro | Value | Meaning |
| --- | --- | --- |
| `SSTEP_OK` | `0` | Success |
| `SSTEP_ERR_PARAM` | `-1` | `NULL` pointer or a value outside its documented range |
| `SSTEP_ERR_CONFIG` | `-2` | Configuration consistent field by field but not as a whole |
| `SSTEP_ERR_STATE` | `-3` | Call not allowed in the current state |
| `SSTEP_ERR_UNSUPPORTED` | `-4` | Topology not compiled in |

### Topologies

| Value | Legs | Meaning |
| --- | --- | --- |
| `SSTEP_TOPOLOGY_FULL_BRIDGE` (0) | 4 | Two H-bridges: phase A between legs 0 and 1, phase B between legs 2 and 3 |
| `SSTEP_TOPOLOGY_SHARED_LEG` (1) | 3 | Three half-bridges: phase A between legs 0 and 2, phase B between legs 1 and 2 |

`SSTEP_LEGS_MAX` (`4U`) is the length of the compare array of `pwm_write()`.

### Fault flags

| Macro | Value | Raised when |
| --- | --- | --- |
| `SSTEP_FAULT_NONE` | `0x0000U` | No fault |
| `SSTEP_FAULT_OVERCURRENT` | `0x0001U` | A phase current above `i_trip_ma` (current drive) |
| `SSTEP_FAULT_OVERVOLTAGE` | `0x0002U` | Bus voltage above `vbus_max_mv` |
| `SSTEP_FAULT_UNDERVOLTAGE` | `0x0004U` | Bus voltage below `vbus_min_mv` |
| `SSTEP_FAULT_HAL` | `0x0008U` | A callback returned an error |
| `SSTEP_FAULT_EXTERNAL` | `0x0010U` | Reported by the application with `sstep_report_fault()` |
| `SSTEP_FAULT_STALL` | `0x0020U` | The rotor does not follow the commanded angle (stall detection, `stall_time_ms` > 0); since 1.1.0. Also together with `SSTEP_FAULT_OVERCURRENT` when the trip comes while the detector is counting |

### Driver states — `enum sstep_state`

| Value | Meaning |
| --- | --- |
| `SSTEP_STATE_DISABLED` (0) | Outputs off |
| `SSTEP_STATE_HOLDING` (1) | Outputs on, position held |
| `SSTEP_STATE_MOVING` (2) | A move or a velocity command is executing |
| `SSTEP_STATE_FAULT` (3) | Outputs off after a fault; leave with `sstep_clear_faults()` |

## Types — `sstep_ctrl.h`

### Callbacks

| Type | Signature | Called from |
| --- | --- | --- |
| `sstep_pwm_write_fn` | `int (*)(void* context, const uint16_t compare[SSTEP_LEGS_MAX])` | `sstep_fast_loop()` |
| `sstep_output_enable_fn` | `int (*)(void* context, bool enable)` | Enable, disable and fault paths |
| `sstep_fault_fn` | `void (*)(void* context, uint16_t faults)` | The context that detects a fault |
| `sstep_event_fn` | `void (*)(void* context, enum sstep_event event)` | `sstep_slow_loop()` |

`pwm_write` and `output_enable` return 0 on success and a negative value on error
(`SSTEP_FAULT_HAL`). Events (`enum sstep_event`): `SSTEP_EVENT_MOVE_DONE` (0, target
reached with zero velocity), `SSTEP_EVENT_HOLD_CURRENT` (1, current reduced at
standstill).

### Configuration

| Type | Members |
| --- | --- |
| `struct sstep_config` | `motor`, `stage`, `ctrl`, `hal` |
| `struct sstep_motor_cfg` | `r_mohm`, `l_uh`, `i_rated_ma`, `ke_uv_s_per_rad`, `full_steps_per_rev` |
| `struct sstep_stage_cfg` | `topology`, `vbus_nominal_mv`, `vbus_min_mv`, `vbus_max_mv`, `i_full_scale_ma`, `i_trip_ma`, `pwm_period_ticks`, `pwm_freq_hz`, `dead_time_ns`, `adc_i_zero`, `adc_i_span`, `adc_vbus_mv_full`, `adc_full` |
| `struct sstep_ctrl_cfg` | `drive_mode`, `fast_loop_hz`, `slow_loop_hz`, `microsteps`, `current_bw_hz`, `i_run_ma`, `i_hold_ma`, `hold_delay_ms`, `v_max`, `accel`, `decel`, `stall_time_ms`, `stall_filter_us`, `stall_speed`, `stall_sync_pct` (since 1.1.0) |
| `struct sstep_callbacks` | `context`, `pwm_write`, `output_enable`, `on_fault`, `on_event` |
| `enum sstep_drive_mode` | `SSTEP_DRIVE_CURRENT` (0), `SSTEP_DRIVE_VOLTAGE` (1) |

Units, ranges and examples of every member: [Run-time configuration](configuration.md#run-time-configuration).

### Measurement and status

| Type | Member | Unit |
| --- | --- | --- |
| `struct sstep_sample` | `i_a`, `i_b` | Phase currents, ADC counts (ignored in voltage drive) |
| | `vbus` | Bus voltage, ADC counts |
| `struct sstep_status` | `state`, `faults` | `enum sstep_state`, fault flags |
| | `position`, `target` | microsteps |
| | `velocity` | microsteps/s (commanded) |
| | `i_a_ma`, `i_b_ma` | mA (last measured) |
| | `vbus_mv` | mV (last measured) |
| | `emf_d_mv`, `emf_q_mv` | mV, filtered back-EMF estimate along and 90° ahead of the commanded current (since 1.1.0) |
| | `rotor_velocity` | microsteps/s, estimated from the back-EMF (since 1.1.0) |
| | `load_angle_cdeg` | 0.01°, estimated load angle, commanded minus rotor electrical angle (since 1.1.0) |
| | `stall_level_pct` | %, stall detector level, 100 = trip (since 1.1.0) |

The five estimates are 0 while stall detection is off; their equations are in
[Stall detection](motor-control.md#stall-detection).

### Driver instance

`struct sstep` is an opaque array of `SSTEP_INSTANCE_WORDS` 64-bit words (on Cortex-M4
432 B fixed point, 456 B float; on 64-bit hosts 456 B and 480 B). Allocate it, pass its
address, never read or write its content. `SSTEP_CONFIG_BYTES` is the size of the
configuration copy (152 B with 32-bit pointers, 176 B with 64-bit pointers) and
`SSTEP_STATE_BYTES` the size of the private state behind it.

## Functions — `sstep.h`

All functions return a status code. Positions are in microsteps, velocities in
microsteps/s, currents in mA. Every function except `sstep_init()` returns
`SSTEP_ERR_STATE` for an instance without a successful `sstep_init()` and then calls
no callback (`sstep_fast_loop()` only with `SSTEP_CFG_ARG_CHECKS = 1`); the tables
below list the other cases.

### sstep_init

```c
int sstep_init(struct sstep* drv, const struct sstep_config* cfg);
```

Validate a configuration and initialize a driver; the outputs stay off. Derives the
per-unit bases, the PI gains ($K_p = L\,\omega_c$, $K_i = R\,\omega_c$), the trajectory constants and,
with `stall_time_ms` > 0, the stall detector constants without floating point. Calls no
callback.

| Parameter | Description |
| --- | --- |
| `drv` | Driver instance |
| `cfg` | Configuration; copied |

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Success, state `SSTEP_STATE_DISABLED` |
| `SSTEP_ERR_PARAM` | `NULL` pointer, missing required callback or a field out of range |
| `SSTEP_ERR_CONFIG` | Inconsistent fields (for example `i_run_ma > i_trip_ma`, stall detection with voltage drive or without `ke_uv_s_per_rad`) or a gain that does not fit |
| `SSTEP_ERR_UNSUPPORTED` | Topology not compiled in |

### sstep_enable

```c
int sstep_enable(struct sstep* drv);
```

Switch the outputs on and hold the current position with the run current; resets the
current controllers. Does nothing when already enabled.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Enabled (`SSTEP_STATE_HOLDING`) |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |
| `SSTEP_ERR_STATE` | Faulted, or `output_enable()` failed (then `SSTEP_FAULT_HAL`) |

### sstep_disable

```c
int sstep_disable(struct sstep* drv);
```

Switch the outputs off at once (the motor coasts, no holding torque) and cancel the
command. A faulted driver stays faulted.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Disabled |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |

### sstep_fast_loop

```c
int sstep_fast_loop(struct sstep* drv, const struct sstep_sample* sample);
```

Convert the measurements, check the trip levels, run the current controllers, limit the
voltage, compute the leg duty cycles and call `pwm_write()`. Call from the PWM/ADC
interrupt at `fast_loop_hz`. The measurements are stored in every state; control and
protection run only while the outputs are on.

| Parameter | Description |
| --- | --- |
| `drv` | Driver instance |
| `sample` | Raw ADC readings of this period |

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done (faults are reported through the state and `on_fault()`) |
| `SSTEP_ERR_PARAM` | `NULL` argument; only with `SSTEP_CFG_ARG_CHECKS = 1` |

### sstep_slow_loop

```c
int sstep_slow_loop(struct sstep* drv);
```

Advance the trajectory by one period, prepare the setpoint of the fast loop, switch
between run and hold current, compensate the bus voltage, update the state and report
events. Call at `slow_loop_hz`.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |

### sstep_move_to

```c
int sstep_move_to(struct sstep* drv, int32_t position);
```

Move to an absolute position with the trapezoidal profile. Replaces the running command
without a velocity step.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Accepted |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |
| `SSTEP_ERR_STATE` | Disabled or faulted |

### sstep_move_by

```c
int sstep_move_by(struct sstep* drv, int32_t delta);
```

Move by `delta` relative to the current target.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Accepted |
| `SSTEP_ERR_PARAM` | `drv` is `NULL`, or the new target is outside `int32_t` |
| `SSTEP_ERR_STATE` | Disabled or faulted |

### sstep_run

```c
int sstep_run(struct sstep* drv, int32_t velocity);
```

Accelerate or decelerate to a signed constant velocity and keep it until another
command.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Accepted |
| `SSTEP_ERR_PARAM` | `drv` is `NULL`, or `|velocity| > v_max` |
| `SSTEP_ERR_STATE` | Disabled or faulted |

### sstep_stop

```c
int sstep_stop(struct sstep* drv);
```

Decelerate to standstill with the configured deceleration and hold there. Does nothing
when the motor is not moving.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Accepted or nothing to stop |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |

### sstep_set_position

```c
int sstep_set_position(struct sstep* drv, int32_t position);
```

Give the current position a new value (homing). The electrical angle is kept, so the
rotor does not move.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |
| `SSTEP_ERR_STATE` | A command is executing |

### sstep_set_current

```c
int sstep_set_current(struct sstep* drv, uint32_t run_ma, uint32_t hold_ma);
```

Change the run and hold currents; used from the next slow period.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL`, `run_ma` is 0 or a value above $10^7$ |
| `SSTEP_ERR_CONFIG` | `hold_ma > run_ma`, or `run_ma` above `i_trip_ma` or `i_rated_ma` |

### sstep_set_motion_limits

```c
int sstep_set_motion_limits(struct sstep* drv, uint32_t v_max, uint32_t accel, uint32_t decel);
```

Change the velocity limit, the acceleration and the deceleration (`decel = 0`: same as
`accel`). They apply from the next slow period, also to a running command.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL`, or a value that rounds to zero or is too large ([ranges](configuration.md#control-and-motion)) |

### sstep_report_fault

```c
int sstep_report_fault(struct sstep* drv, uint16_t faults);
```

Raise fault flags detected by the application (gate driver fault pin, temperature): the
outputs go off at once and `on_fault()` is called. May be called from an interrupt.
`SSTEP_FAULT_NONE` does nothing.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |

### sstep_clear_faults

```c
int sstep_clear_faults(struct sstep* drv);
```

Clear all fault flags; the driver goes to `SSTEP_STATE_DISABLED` and can be enabled again.

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | Done |
| `SSTEP_ERR_PARAM` | `drv` is `NULL` |

### sstep_get_status

```c
int sstep_get_status(const struct sstep* drv, struct sstep_status* status);
```

Read a snapshot of the state, faults, commanded motion, the last measurements and the
stall detector estimates in engineering units. With stall detection on it also converts
the back-EMF estimate (a square root and a CORDIC arctangent, about 1200 instructions on
Cortex-M4): call it from the slow-loop context, not from the fast loop.

| Parameter | Description |
| --- | --- |
| `drv` | Driver instance |
| `status` | Filled on success |

| Return | Meaning |
| --- | --- |
| `SSTEP_OK` | `status` filled |
| `SSTEP_ERR_PARAM` | A `NULL` pointer |

## Return codes summary

| Function | `OK` | `ERR_PARAM` | `ERR_CONFIG` | `ERR_STATE` | `ERR_UNSUPPORTED` |
| --- | :---: | :---: | :---: | :---: | :---: |
| `sstep_init()` | ✓ | ✓ | ✓ | | ✓ |
| `sstep_enable()` | ✓ | ✓ | | ✓ | |
| `sstep_disable()` | ✓ | ✓ | | ✓¹ | |
| `sstep_fast_loop()` | ✓ | ✓ | | ✓¹ | |
| `sstep_slow_loop()` | ✓ | ✓ | | ✓¹ | |
| `sstep_move_to()`, `sstep_move_by()`, `sstep_run()` | ✓ | ✓ | | ✓ | |
| `sstep_stop()` | ✓ | ✓ | | ✓¹ | |
| `sstep_set_position()` | ✓ | ✓ | | ✓ | |
| `sstep_set_current()` | ✓ | ✓ | ✓ | ✓¹ | |
| `sstep_set_motion_limits()` | ✓ | ✓ | | ✓¹ | |
| `sstep_report_fault()`, `sstep_clear_faults()`, `sstep_get_status()` | ✓ | ✓ | | ✓¹ | |

¹ Only for an instance without a successful `sstep_init()`.
