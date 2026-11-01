# Extending the Library

Most extensions are built on top of Stepper-C without changing it: homing, limit
switches, communication, multi-axis coordination and encoders are application code that
calls the public API. This chapter shows where each extension belongs and the rules for
changes to the library itself.

## Where to extend

| You want to… | Do it in | Library change needed? |
| --- | --- | --- |
| Drive another MCU or board | Your `pwm_write()`, `output_enable()`, ADC and interrupt code | No |
| Home the axis on a switch | Application: `sstep_run()` towards the switch, `sstep_stop()` on the edge, `sstep_set_position(0)` | No |
| Add soft limits | Application: clamp targets before `sstep_move_to()` | No |
| Accept commands over UART, CAN, Modbus | Application: protocol handler that calls the commands from the slow-loop context | No |
| Coordinate several axes | Application: one instance per axis, start moves in the same slow period, scale `v_max` per axis | No |
| Detect a stall without a sensor | Configuration: `stall_time_ms` and the other `stall_*` fields ([Stall detection](motor-control.md#stall-detection)) | No |
| Detect a stall with an encoder | Application: compare the encoder with `sstep_get_status()` position, `sstep_report_fault()` on a large difference | No |
| Report a temperature or driver fault | `sstep_report_fault()` from the interrupt or task that detects it | No |
| Use another numeric format or table size | Build-time options in `sstep_user_config.h` | No |
| Support a new power stage topology | `src/sstep_bridge.c` (limit and duty cycles), `enum sstep_topology`, a build-time option | Yes |
| A different motion profile (S-curve) | `src/sstep_motion.c` | Yes |
| Dead-time compensation | Fast loop; the configuration already has `dead_time_ns` | Yes |

## Homing example

```c
enum homing_state { HOMING_SEEK, HOMING_STOP, HOMING_DONE };

static enum homing_state homing = HOMING_SEEK;

/* called from the slow-loop context after sstep_slow_loop() */
void homing_step(bool switch_pressed)
{
    struct sstep_status st;
    switch (homing)
    {
    case HOMING_SEEK:
        if (switch_pressed)
        {
            (void)sstep_stop(&motor);
            homing = HOMING_STOP;
        }
        break;
    case HOMING_STOP:
        if ((sstep_get_status(&motor, &st) == SSTEP_OK) && (st.state == SSTEP_STATE_HOLDING))
        {
            (void)sstep_set_position(&motor, 0);
            homing = HOMING_DONE;
        }
        break;
    default:
        break;
    }
}
```

Start it with `sstep_run(&motor, -1600)` (towards the switch, at a low speed). The stop
uses the configured deceleration, so the zero is where the motor came to rest, a few
microsteps after the switch; for a repeatable zero, move back off the switch slowly and
take the edge again.

## Changing the configuration

Build-time options go into `sstep_user_config.h` in a folder of your project:

```c
/* sstep_user_config.h */
#define SSTEP_CFG_MATH_BACKEND SSTEP_MATH_BACKEND_FLOAT
#define SSTEP_CFG_TOPOLOGY_FULL_BRIDGE 0U /* only the 3-leg stage of this board */
#define SSTEP_CFG_MICROSTEPS_MAX 64U
```

and CMake gets `-DSSTEP_CONFIG_DIR=<folder>`. Run-time values (motor, currents, limits)
need no rebuild: pass another `struct sstep_config` to `sstep_init()`, or use
`sstep_set_current()` and `sstep_set_motion_limits()` while running.

## Contributing to the library

1. **Code.** C11, no new dependencies, no heap, no global state; the rules of
   `misra-analyze`; internal types stay in `src/`, `sstep.h` declares the public API only.
2. **Both backends.** Every arithmetic change works with fixed point and float; the
   `[UnitTests]` and `[UnitTestsFloat]` presets must both pass.
3. **Tests.** 100 % line and branch coverage of `src/`; a closed-loop test on the motor
   model for any change of the control behavior.
4. **Instance size.** If the private state grows, update `SSTEP_STATE_BYTES` in
   `inc/sstep_ctrl.h`; the build fails until it fits.
5. **Analysis.** `misra-analyze` 0 violations, or a recorded deviation in
   [MISRA C:2012](misra.md).
6. **Format.** `clang-format` reports no changes in the files you touched.
7. **Documentation.** The affected pages, the stack table, the diagrams and
   `CHANGELOG.md` `[Unreleased]`.
8. **Compatibility.** The public API is stable within a major version.
