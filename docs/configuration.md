# Configuration

Stepper-C has two kinds of configuration. **Build-time options** select the code that is
compiled in (numeric backend, topologies, table size); they are macros in
`inc/sstep_config.h`. **Run-time configuration** describes the motor, the power stage and
the control; it is a `struct sstep_config` passed to `sstep_init()`. This chapter lists
both, then the memory footprint and the measured stack usage.

## Build-time options

Each macro has a default, is range-checked with `#error` and can be overridden on the
compiler command line or in a header `sstep_user_config.h` (included when
`SSTEP_USER_CONFIG` is defined; with CMake, set `SSTEP_CONFIG_DIR` to its folder).
The application must be compiled with the same values as the library: they change the
size of `struct sstep`.

| Macro | Default | Range | Meaning |
| --- | --- | --- | --- |
| `SSTEP_CFG_MATH_BACKEND` | `SSTEP_MATH_BACKEND_FIXED` | `…_FIXED`, `…_FLOAT` | Numeric backend (CMake: `SSTEP_MATH_BACKEND=FIXED` or `FLOAT`) |
| `SSTEP_CFG_TOPOLOGY_FULL_BRIDGE` | `1` | 0, 1 | Compile the two-H-bridge topology |
| `SSTEP_CFG_TOPOLOGY_SHARED_LEG` | `1` | 0, 1 | Compile the three-leg topology; at least one must be 1 |
| `SSTEP_CFG_MICROSTEPS_MAX` | `256U` | power of two, 1 .. 256 | Largest `microsteps` accepted by `sstep_init()` |
| `SSTEP_CFG_SINE_TABLE_BITS` | `8U` | 6 .. 10 | Quarter-wave table of $2^\mathrm{bits} + 1$ entries (see the [accuracy table](motor-control.md#microstepping)) |
| `SSTEP_CFG_ARG_CHECKS` | `1` | 0, 1 | 0 removes the `NULL` checks from `sstep_fast_loop()` only |

## Run-time configuration

`struct sstep_config` has four parts. All values are integers in engineering units;
`sstep_init()` copies the structure, so it may be a temporary or a `const` in flash.
Fields outside their range make `sstep_init()` return `SSTEP_ERR_PARAM`; combinations
that do not fit return `SSTEP_ERR_CONFIG`.

### Motor

| Field | Unit | Range | 17HS19-2004S1 | Where to get it |
| --- | --- | --- | --- | --- |
| `r_mohm` | mΩ | 1 .. $10^7$ | 1600 | Datasheet (phase resistance at 25 °C) or an ohmmeter |
| `l_uh` | µH | 1 .. $10^7$ | 3000 | Datasheet (phase inductance) or an LCR meter at 1 kHz |
| `i_rated_ma` | mA | 1 .. $10^7$ | 2000 | Datasheet; `i_run_ma` may not exceed it |
| `ke_uv_s_per_rad` | µV·s/rad | 0 .. $10^7$ | 208600 | $T_\mathrm{hold} / (\sqrt{2}\, I_\mathrm{rated})$ in µN·m/A ($0.59 / (\sqrt{2} \cdot 2) = 0.2086$ N·m/A); 0 disables the feed-forward |
| `full_steps_per_rev` | – | ≥ 1 | 200 | 200 for 1.8°, 400 for 0.9° |

### Power stage and measurement

| Field | Unit | Range | B-G431B-ESC1 example | Meaning |
| --- | --- | --- | --- | --- |
| `topology` | – | compiled-in value | `SSTEP_TOPOLOGY_SHARED_LEG` | See [Voltage limits](motor-control.md#voltage-limits) |
| `vbus_nominal_mv` | mV | 1 .. $10^7$ | 24000 | Nominal supply; base of the per-unit voltage |
| `vbus_min_mv` | mV | 0 or below nominal | 10000 | Undervoltage trip; 0 disables it |
| `vbus_max_mv` | mV | 0 or above nominal | 28000 | Overvoltage trip; 0 disables it |
| `i_full_scale_ma` | mA | 1 .. $10^7$ | 4000 | Phase current at the ADC reading `adc_i_zero + adc_i_span`; base of the per-unit current |
| `i_trip_ma` | mA | 1 .. `i_full_scale_ma` | 3500 | Overcurrent trip (either polarity) |
| `pwm_period_ticks` | timer ticks | ≥ 2 | 4250 | Compare value for 100 % duty cycle (timer auto-reload) |
| `pwm_freq_hz` | Hz | – | 20000 | Reserved; not used by this version |
| `dead_time_ns` | ns | – | 0 | Reserved for dead-time compensation; not used by this version |
| `adc_i_zero` | counts | ≤ `adc_full` | 2048 | Current ADC reading at 0 A (measure with the outputs off) |
| `adc_i_span` | counts | ±1 .. ±65535 | 2047 | Counts from 0 A to +`i_full_scale_ma`; negative for an inverting amplifier |
| `adc_vbus_mv_full` | mV | 1 .. $10^7$ | 34320 | Bus voltage that gives the reading `adc_full` (divider ratio × ADC reference) |
| `adc_full` | counts | ≥ 1 | 4095 | Largest ADC reading |

The B-G431B-ESC1 values above are placeholders derived from the schematic; measure
`adc_i_zero`, `adc_i_span` and `adc_vbus_mv_full` on your board.

### Control and motion

| Field | Unit | Range | Example | Meaning |
| --- | --- | --- | --- | --- |
| `drive_mode` | – | `SSTEP_DRIVE_CURRENT`, `SSTEP_DRIVE_VOLTAGE` | current | Closed-loop current or open-loop voltage drive |
| `fast_loop_hz` | Hz | 1 .. $10^7$ | 20000 | Rate of `sstep_fast_loop()` |
| `slow_loop_hz` | Hz | divides `fast_loop_hz`, ratio ≤ 65535 | 1000 | Rate of `sstep_slow_loop()` |
| `microsteps` | – | power of two, 1 .. `SSTEP_CFG_MICROSTEPS_MAX` | 16 | Microsteps per full step |
| `current_bw_hz` | Hz | 1 .. `fast_loop_hz / 4` | 1000 | Current loop bandwidth; the PI gains follow from it |
| `i_run_ma` | mA | 1 .. min(`i_trip_ma`, `i_rated_ma`) | 1500 | Peak phase current while moving |
| `i_hold_ma` | mA | 0 .. `i_run_ma` | 500 | Peak phase current at standstill |
| `hold_delay_ms` | ms | any | 500 | Standstill time before the hold current |
| `v_max` | microsteps/s | see below | 16000 | Velocity limit |
| `accel` | microsteps/s² | see below | 64000 | Acceleration |
| `decel` | microsteps/s² | as `accel`, or 0 | 0 | Deceleration; 0 = same as `accel` |

The motion limits are converted to 1/65 536 microstep per slow period. Each must round
to at least 1 and stay below $2^{30}$: with `slow_loop_hz = 1000`, `v_max` from 0.008 to
below 16 384 000 microsteps/s, `accel` and `decel` from 8 to below
$1.638 \cdot 10^{10}$ microsteps/s².

### Callbacks

| Field | Required | Called from | Contract |
| --- | --- | --- | --- |
| `context` | – | – | Passed unchanged to every callback |
| `pwm_write` | Yes | `sstep_fast_loop()` | Load one compare value per leg (`compare[0..3]`, `compare[3]` is 0 for the shared leg) for the next PWM period; return 0, or a negative value on error |
| `output_enable` | Yes | Any function that enables, disables or faults | true: enable the gate drivers; false: all legs high impedance. Return 0, or a negative value on error |
| `on_fault` | No | The context that detects the fault (may be the interrupt) | Informs the application of new fault flags; outputs are already off |
| `on_event` | No | `sstep_slow_loop()` | `SSTEP_EVENT_MOVE_DONE`, `SSTEP_EVENT_HOLD_CURRENT` |

## Memory footprint

Measured with `arm-none-eabi-gcc` 15.2.1 for Cortex-M4 (`-mthumb`, float backend with
`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`), default build-time options, both topologies.

| Object | Fixed point | Float |
| --- | --- | --- |
| `struct sstep` (RAM per motor) | 336 B | 360 B |
| Code and constants, `-Os` (flash) | 6980 B | 7446 B |
| of which the sine table | 514 B | 1028 B |
| `.data` + `.bss` | 0 B | 0 B |

`struct sstep` is the configuration copy (`SSTEP_CONFIG_BYTES`: 136 B on Cortex-M4,
160 B on 64-bit hosts) plus the private state (`SSTEP_STATE_BYTES`: 200 B fixed point,
224 B float on Cortex-M4, 232 B float on 64-bit hosts); on a 64-bit host the instance
is 360 B and 392 B.

## Stack usage

Worst-case stack of each public function including the library functions it calls,
measured with `-fstack-usage -fcallgraph-info=su` and `tools/docs/stack_usage.py`
(Cortex-M4, toolchain as above). Calls through the callbacks are not included: add the
stack of your `pwm_write()`, `output_enable()`, `on_fault()` and `on_event()` to the
functions that call them.

| Function | Fixed `-Os` | Fixed `-O0` | Float `-Os` | Float `-O0` |
| --- | ---: | ---: | ---: | ---: |
| `sstep_init()` | 192 | 1096 | 168 | 1064 |
| `sstep_enable()` | 48 | 80 | 64 | 88 |
| `sstep_disable()` | 24 | 40 | 24 | 40 |
| `sstep_fast_loop()` | 184 | 304 | 128 | 312 |
| `sstep_slow_loop()` | 140 | 560 | 132 | 552 |
| `sstep_move_to()` | 0 | 48 | 0 | 48 |
| `sstep_move_by()` | 0 | 112 | 0 | 112 |
| `sstep_run()` | 16 | 104 | 16 | 104 |
| `sstep_stop()` | 0 | 40 | 0 | 40 |
| `sstep_set_position()` | 16 | 72 | 16 | 72 |
| `sstep_set_current()` | 52 | 216 | 36 | 128 |
| `sstep_set_motion_limits()` | 72 | 224 | 72 | 224 |
| `sstep_report_fault()` | 24 | 40 | 24 | 40 |
| `sstep_clear_faults()` | 0 | 40 | 0 | 40 |
| `sstep_get_status()` | 24 | 184 | 16 | 152 |

The interrupt stack must hold the frame of `sstep_fast_loop()` plus `pwm_write()` (and
`output_enable()` + `on_fault()` for the fault path) plus the interrupt entry. There is
no recursion; every frame is static.

To re-measure:

```bash
mkdir stack && cd stack
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -std=c11 -fstack-usage -fcallgraph-info=su \
    -I.. -I../inc -c ../src/*.c
python ../tools/docs/stack_usage.py . --markdown
```

## CMake options

| Option | Default | Meaning |
| --- | --- | --- |
| `SSTEP_MATH_BACKEND` | `FIXED` | Numeric backend: `FIXED` or `FLOAT` |
| `SSTEP_CONFIG_DIR` | empty | Folder with `sstep_user_config.h` |
| `SSTEP_UNIT_TESTS` | `OFF` | GoogleTest suite and `coverage` target |
| `SSTEP_COVERAGE_MIN` | `100` | Coverage gate (lines and branches, %) |
| `SSTEP_MISRA_FAIL_ON_VIOLATION` | `ON` | `misra-analyze` fails on any violation |
| `SSTEP_MISRA_RULE_TEXTS` | empty | Licensed MISRA rule texts for readable reports |
