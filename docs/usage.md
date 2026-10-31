# Using the Library

This chapter shows how to bring a motor up with Stepper-C: fill the configuration, start
the driver, command motion, read the status and handle faults. The hardware side (timer,
ADC, interrupts) is in the [Integration Guide](integration.md); the meaning of every
configuration field is in [Configuration](configuration.md#run-time-configuration).

## Setting up a driver

One `struct sstep` controls one motor. Allocate it statically (or in any memory that
lives as long as the motor runs), fill a `struct sstep_config` and call `sstep_init()`:

```c
static struct sstep motor;

int motor_start(const struct sstep_config* cfg)
{
    int result = sstep_init(&motor, cfg);
    if (result == SSTEP_OK)
    {
        result = sstep_enable(&motor);
    }
    return result;
}
```

`sstep_init()` checks every field, derives the controller gains, the ADC scaling, the
trip levels and the motion constants, and leaves the outputs off
(`SSTEP_STATE_DISABLED`). It does not call any callback. Start the PWM timer and the ADC
before `sstep_enable()`: the fast loop must already run when the outputs are switched on.

`sstep_enable()` calls `output_enable(true)` and holds the current position with the run
current (the hold current follows after `hold_delay_ms`). The rotor jumps to the nearest
position of the commanded electrical angle when the current appears: up to two full
steps on the first enable after power-up.

## Commanding motion

All motion commands take positions in microsteps and return immediately; the slow loop
executes them. They are accepted in `SSTEP_STATE_HOLDING` and `SSTEP_STATE_MOVING`.

| Command | Effect |
| --- | --- |
| `sstep_move_to(&motor, 3200)` | Move to the absolute position 3200 with the trapezoidal profile |
| `sstep_move_by(&motor, -800)` | Move 800 microsteps back from the current *target* |
| `sstep_run(&motor, 6400)` | Accelerate to 6400 microsteps/s and keep running (negative: other direction) |
| `sstep_stop(&motor)` | Decelerate to standstill and hold |
| `sstep_set_position(&motor, 0)` | Call the current position 0 (homing); only at standstill, the rotor does not move |
| `sstep_set_motion_limits(&motor, v_max, accel, decel)` | New limits, used from the next slow period |
| `sstep_set_current(&motor, run_ma, hold_ma)` | New run and hold currents |

A command replaces the running one without a velocity step: a new target during a move,
a run during a move, a reversal during a run. `sstep_move_by()` adds to the target, so
several relative moves given in a row add up.

Wait for a move with the event callback or by polling the status:

```c
static volatile bool move_done;

static void on_event(void* context, enum sstep_event event)
{
    (void)context;
    if (event == SSTEP_EVENT_MOVE_DONE)
    {
        move_done = true;
    }
}

bool motor_is_idle(void)
{
    struct sstep_status st;
    return (sstep_get_status(&motor, &st) == SSTEP_OK) && (st.state == SSTEP_STATE_HOLDING);
}
```

## Reading the status

`sstep_get_status()` fills a snapshot in engineering units:

| Field | Unit | Meaning |
| --- | --- | --- |
| `state` | – | `SSTEP_STATE_DISABLED`, `…_HOLDING`, `…_MOVING`, `…_FAULT` |
| `faults` | flags | Active `SSTEP_FAULT_…` flags |
| `position` | microsteps | Commanded position (rounded) |
| `target` | microsteps | Target of the current move |
| `velocity` | microsteps/s | Commanded velocity |
| `i_a_ma`, `i_b_ma` | mA | Last measured phase currents |
| `vbus_mv` | mV | Last measured bus voltage |

Position and velocity are *commanded* values. Without an encoder the library cannot see
a lost step; keep the load angle reserve with enough current and a sensible acceleration
([Motor Control](motor-control.md#motor-model)).

## Handling faults

A fault switches the outputs off at once, sets `SSTEP_STATE_FAULT` and calls
`on_fault()` with the new flags. Commands then return `SSTEP_ERR_STATE`.

| Flag | Raised by | Typical cause |
| --- | --- | --- |
| `SSTEP_FAULT_OVERCURRENT` | Fast loop, current drive only | Short circuit, wrong `adc_i_span` sign, gains far too high |
| `SSTEP_FAULT_OVERVOLTAGE` | Fast loop | Regenerated energy when braking a load; supply too high |
| `SSTEP_FAULT_UNDERVOLTAGE` | Fast loop | Supply off or sagging; `vbus_min_mv` too close to nominal |
| `SSTEP_FAULT_HAL` | Any function that calls a callback | `pwm_write()` or `output_enable()` returned a negative value |
| `SSTEP_FAULT_EXTERNAL` (or any flag you choose) | `sstep_report_fault()` | Gate driver fault pin, over-temperature |

Report a gate driver fault from its interrupt:

```c
void EXTI15_10_IRQHandler(void);
void EXTI15_10_IRQHandler(void)
{
    if (board_driver_fault())
    {
        (void)sstep_report_fault(&motor, SSTEP_FAULT_EXTERNAL);
    }
}
```

To recover, remove the cause, then:

```c
if ((sstep_clear_faults(&motor) == SSTEP_OK) && (sstep_enable(&motor) == SSTEP_OK))
{
    board_led(false);
}
```

`sstep_clear_faults()` returns to `SSTEP_STATE_DISABLED`; the position is kept, so the
motor continues from where it was commanded (it may have been moved by hand meanwhile).

## Several motors

Each motor has its own instance and its own callbacks; use `context` to tell the
callbacks which hardware they drive:

```c
static struct sstep axes[2];
static const uint32_t axis_ids[2] = {0U, 1U};

static int axis_pwm_write(void* context, const uint16_t compare[SSTEP_LEGS_MAX])
{
    board_pwm_set_axis(*(const uint32_t*)context, compare);
    return 0;
}
```

with `.hal.context = (void*)&axis_ids[k]` in the configuration of axis `k`. The fast loops
of several axes may run in one interrupt one after the other, or in separate interrupts.

## Return values

| Code | Value | Returned when |
| --- | --- | --- |
| `SSTEP_OK` | 0 | Success |
| `SSTEP_ERR_PARAM` | −1 | `NULL` pointer, a value outside its range, `|velocity| > v_max`, a relative move that overflows |
| `SSTEP_ERR_CONFIG` | −2 | Configuration fields that do not fit together, currents above the trip or rated current |
| `SSTEP_ERR_STATE` | −3 | A motion command while disabled or faulted, `sstep_set_position()` while moving, `sstep_enable()` while faulted |
| `SSTEP_ERR_UNSUPPORTED` | −4 | A topology that is not compiled in |

`sstep_fast_loop()` returns `SSTEP_ERR_PARAM` only for `NULL` arguments and only with
`SSTEP_CFG_ARG_CHECKS = 1`; its faults are reported through the state and `on_fault()`.

## Common mistakes

| Symptom | Likely cause |
| --- | --- |
| `SSTEP_FAULT_OVERCURRENT` right after `sstep_enable()` | `adc_i_span` has the wrong sign (inverting amplifier) or `adc_i_zero` is wrong: the controller sees the current in the wrong direction and drives it up |
| `SSTEP_FAULT_UNDERVOLTAGE` right after `sstep_enable()` | The fast loop ran before the first ADC conversion (`vbus` = 0), or `adc_vbus_mv_full` is wrong |
| The motor hums but does not turn | Phases swapped or one phase reversed; `current_bw_hz` too low; `v_max` above what the supply can drive |
| The motor turns the wrong way | Swap the two wires of one phase, or negate positions in the application |
| Loses steps at speed | Not enough voltage for the speed ([Voltage limits](motor-control.md#voltage-limits)); shared-leg stage at its limit; acceleration too high for the load |
| Moves are jerky, about once per millisecond | `sstep_slow_loop()` is not called at `slow_loop_hz`, or `fast_loop_hz` does not match the real interrupt rate |
| Current settles at the wrong value | `i_full_scale_ma` / `adc_i_span` do not match the shunt and amplifier |
| `sstep_init()` returns `SSTEP_ERR_CONFIG` | Check the combinations: trip ≤ full scale, run current ≤ trip and rated, bandwidth ≤ `fast_loop_hz / 4`, `fast_loop_hz` a multiple of `slow_loop_hz` |
