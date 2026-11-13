# Protection

Stepper-C switches the outputs off in the context that detects a problem, within the fast
loop period in which the sample shows it. This page lists every protection of the library,
what it detects, when it acts and how the application sees it, and what the firmware around
the library has to add.

## Overview

| Protection | Detects | Acts | Reaction | Reported as |
| --- | --- | --- | --- | --- |
| Phase overcurrent | $\lvert i_A \rvert$ or $\lvert i_B \rvert$ above `i_trip_ma` | Every fast loop period: current drive, voltage drive with `i_trip_voltage_drive`, during the self-test | Outputs off in the same period, fault latched | `SSTEP_FAULT_OVERCURRENT` (with `SSTEP_FAULT_STALL` while the stall detector counts) |
| Shared-leg overcurrent | $\lvert i_A + i_B \rvert$ above `i_trip_ma`: the current of the common leg | As above, `SSTEP_TOPOLOGY_SHARED_LEG` only | As above | `SSTEP_FAULT_OVERCURRENT` |
| Overvoltage, undervoltage | Bus voltage above `vbus_max_mv` or below `vbus_min_mv` | Every fast loop period with the outputs on | As above | `SSTEP_FAULT_OVERVOLTAGE`, `SSTEP_FAULT_UNDERVOLTAGE` |
| Bus voltage at the start | The last bus voltage sample outside the trip limits | `sstep_enable()`, `sstep_selftest()` | The call is refused, nothing is switched on, nothing latched | `SSTEP_ERR_VBUS` |
| Self-test | A switch that conducts, an open phase, a short, a current of the wrong sign, a phase on the wrong legs | `sstep_selftest()`, before the first enable | Outputs off, fault latched | `SSTEP_FAULT_SELFTEST`, result in `sstep_status::selftest` |
| Stall detection | The rotor does not follow the commanded angle | Slow loop, after `stall_time_ms` | Outputs off, fault latched | `SSTEP_FAULT_STALL` |
| Callback errors | `pwm_write()` or `output_enable()` returned an error | Where the callback is called | Outputs off, fault latched | `SSTEP_FAULT_HAL` |
| External faults | Anything the application detects: a driver fault pin, the temperature, a hardware comparator | `sstep_report_fault()`, any context, also an interrupt | Outputs off, fault latched | The flags passed, typically `SSTEP_FAULT_EXTERNAL` |

Two configuration options keep the trips from acting on false data or from being caused by
the drive itself:

- **Sampling window** (`sample_window_ticks`): no switching edge falls into the conversions
  of low-side shunts, so a single corrupted sample cannot trip the overcurrent or the
  undervoltage protection ([Sampling window](motor-control.md#sampling-window)).
- **Current slew** (`i_slew_ma_per_ms`): the current rises after `sstep_enable()` instead of
  stepping, so a current-limited supply does not collapse
  ([Current slew](motor-control.md#current-slew)).

## Fault handling

A fault calls `output_enable(false)` in the detecting context, sets the flags in `faults`,
enters `SSTEP_STATE_FAULT` and calls `on_fault()` with the new flags, all before the function
that detected it returns. The outputs then stay off:

- `sstep_enable()`, the motion commands and `sstep_selftest()` answer `SSTEP_ERR_STATE`;
- `sstep_disable()` keeps the fault;
- `sstep_clear_faults()` clears the flags and returns to `SSTEP_STATE_DISABLED`; the
  application decides when that is safe (after a fault of the power stage, usually only
  after a person has looked at it).

`on_fault()` may run in the fast loop interrupt: keep it short and do not call the library
from it. `sstep_get_status()` reports the flags, the state and the last measurements.

## Overcurrent

The trip level `i_trip_ma` is converted once into ADC counts from `adc_i_zero`; the fast
loop compares the raw samples, before any filtering, in every period:

$$
\lvert d_A \rvert > n_\mathrm{trip}, \qquad \lvert d_B \rvert > n_\mathrm{trip}, \qquad
\lvert d_A + d_B \rvert > n_\mathrm{trip}\ \text{(shared leg)}
$$

with $d_x$ the sample minus `adc_i_zero` and $n_\mathrm{trip}$ = `i_trip_ma` ·
`adc_i_span` / `i_full_scale_ma`. The common leg of the shared-leg topology carries
$-(i_A + i_B)$, up to $\sqrt{2}$ times the phase amplitude, so it reaches the switch rating
first.

In `SSTEP_DRIVE_VOLTAGE` the phase currents may not be measured at all, so the trips act
only when `i_trip_voltage_drive` says they are. During `sstep_selftest()` they always act.

The response time is one fast loop period at best. A short circuit through a switch rises
in microseconds: protect the switches in hardware as well (see
[What the application adds](#what-the-application-adds)).

## Bus voltage

`vbus_min_mv` and `vbus_max_mv` trip in every fast loop period while the outputs are on.
Before the outputs are switched on, `sstep_enable()` and `sstep_selftest()` check the last
sample against the same limits and answer `SSTEP_ERR_VBUS` without latching a fault: a supply
that is not yet up is not a fault of the drive, and the call can be repeated once it is.
Without any sample yet `sstep_enable()` does not check (the first fast loop period trips),
`sstep_selftest()` refuses.

## Self-test

`sstep_selftest(drv, i_test_ma)` tests the power stage and the wiring before the first
enable, with the outputs on and the current controller off. It is compiled in by default;
`SSTEP_CFG_SELFTEST = 0` removes it (1.3 KiB of flash and 48 B per instance on Cortex-M4)
and the call returns `SSTEP_ERR_UNSUPPORTED`. The fast loop runs it, one step
per period:

1. **All legs low, 2 ms.** No phase may carry more than a quarter of the test current;
   otherwise a switch conducts with its gate off (`SSTEP_SELFTEST_LEAK`).
2. **Phase A.** The legs of phase B and the return leg of phase A stay low; the duty cycle of
   the driving leg of phase A rises over 20 ms from 0 to $D_\mathrm{max}$ until the current
   reaches `i_test_ma` and is held; then all legs go low until the currents have died out.
3. **Phase B**, the same.

The energised phase pulls the rotor to its nearest full step, at most one full step, and
the rotor swings into it. While it moves, it induces currents in both phases: in the idle
phase, which both its legs short, and on top of the test current. A wrong wiring or a short
gives a current that stays; a rotor comes to rest. So each check waits for a rotor at rest:

| Check | Passes when | Fails after |
| --- | --- | --- |
| Hold (wiring) | The other phase stayed below half the own current for 2 ms in a row | 100 ms without that: `SSTEP_SELFTEST_WRONG_PHASE` |
| Decay (between and after the phases) | Both currents stayed below a quarter of the test current for 5 ms in a row | 100 ms without that: `SSTEP_SELFTEST_LEAK` |
| Twice the test current | A swinging rotor crosses zero every half swing | Above it without a break for 100 ms: `SSTEP_SELFTEST_SHORT` |

With a rotor at rest the conditions hold from the start, and the hold takes 2 ms and the
decay 5 ms. The overcurrent trip (`i_trip_ma`) acts throughout the test, so a hard short
switches the outputs off at once, whatever the checks wait for.

The voltage is ramped and not set from the resistance, because the dead time takes a bus
fraction of $2\, t_\mathrm{dead} f_\mathrm{PWM}$ from a driving leg (0.58 V at 12 V,
1.2 µs and 20 kHz) — as much as the 0.96 V that 0.6 A needs through 1.6 Ω. With
$V_t = I_t R$ (`i_test_ma` · `r_mohm`), the bus voltage $V_\mathrm{bus}$ of the last sample
and the period $P$ in compare ticks:

$$
D_\mathrm{max} = 3\,\frac{V_t}{V_\mathrm{bus}} P + 2\, t_\mathrm{dead} f_\mathrm{PWM} P,
\qquad
D_\mathrm{short} = \frac{V_t}{4 V_\mathrm{bus}} P + 2\, t_\mathrm{dead} f_\mathrm{PWM} P
$$

`dead_time_ns` and `pwm_freq_hz` enter here; with 0 the loss term is 0.

| Result | Condition | Likely cause |
| --- | --- | --- |
| `SSTEP_SELFTEST_PASS` | Both phases reached the test current with the expected sign, the other phase stayed below half of it | — |
| `SSTEP_SELFTEST_LEAK` | Current above a quarter of the test current with every leg low, before the first phase or for 100 ms between the phases | A high-side switch conducts |
| `SSTEP_SELFTEST_OPEN` | $D_\mathrm{max}$ reached without the current | Phase wire loose, motor not connected, a leg or its gate driver dead |
| `SSTEP_SELFTEST_SHORT` | Current above twice the test current for 100 ms, reached below $D_\mathrm{short}$ (resistance below a quarter of `r_mohm`), or an overcurrent trip | Phases shorted together or to a supply rail |
| `SSTEP_SELFTEST_POLARITY` | The test current with the wrong sign | Sign of `adc_i_span` |
| `SSTEP_SELFTEST_WRONG_PHASE` | The other phase carried half the own current or more for 100 ms | A phase connected to the wrong legs |
| `SSTEP_SELFTEST_ABORTED` | A bus voltage trip or a callback error ended the test | Supply, see the fault flags |
| `SSTEP_SELFTEST_NONE` | Not run, or stopped by `sstep_disable()` | — |

`sstep_status` also reports the tested phase (`selftest_phase`), its current at the end
(`selftest_i_ma`) and the compare value (`selftest_duty`). A failure raises
`SSTEP_FAULT_SELFTEST`; the result stays readable after `sstep_clear_faults()`.

The test takes 42 ms on the motor model of the tests (17HS19-2004S1, 12 V, free rotor,
both topologies), and 39 to 55 ms with the rotor a full step away from phase A at 12 V and
24 V and up to 50 times the rotor inertia. A rotor that keeps moving can stretch it to about
440 ms (2 + 2 × (20 + 100 + 100) ms) before it fails. The end is reported by
`sstep_slow_loop()` with `SSTEP_EVENT_SELFTEST_DONE`.

!!! warning "The rotor must be free to settle"
    The energised phase pulls the rotor to its nearest full step (at most one full step)
    and back when the motor is enabled; the checks wait until it is at rest. A rotor turned
    by an external torque for longer, such as a joint that falls, keeps inducing currents
    and fails the test: support such a load for the first enable.

A typical start, in the slow loop context of a bare-metal main loop:

```c
struct sstep_status st = {0};

if (sstep_selftest(&motor, 600U) == SSTEP_OK) /* 30 % of the 2 A rated current */
{
    do
    {
        if (board_tick_1ms())
        {
            (void)sstep_slow_loop(&motor);
        }
        (void)sstep_get_status(&motor, &st);
    } while (st.selftest == SSTEP_SELFTEST_RUNNING);
}
if (st.selftest == SSTEP_SELFTEST_PASS)
{
    (void)sstep_enable(&motor);
}
```

## What the application adds

The library sees one sample per fast loop period and the callbacks. Protections that need
the hardware, faster reaction or information outside the samples belong to the firmware:

| Protection | Why it is outside the library |
| --- | --- |
| Hardware overcurrent (comparator into the break input of the PWM timer) | Acts in microseconds without software; a short through a switch rises faster than one fast loop period |
| Plausibility of `adc_i_zero` (offset calibration with the outputs off) | The calibration is done by the application before `sstep_init()` |
| Current of a third leg, if the board measures it | `struct sstep_sample` has two currents; the shared-leg trip uses $i_A + i_B$ |
| Fast loop overrun | Only the timer or the ADC interrupt sees that the next sample came before the loop ended |
| Temperature of the stage and the motor | Report it with `sstep_report_fault(SSTEP_FAULT_EXTERNAL)` |
| Gate driver fault pins, watchdog | Board and system specific |

## Bring-up

1. With the outputs off, check `adc_i_zero` and the bus voltage reading
   ([Bring-up checklist](integration.md#bring-up-checklist)).
2. Set `sample_window_ticks` for low-side shunts and a moderate `i_slew_ma_per_ms`.
3. Run `sstep_selftest()` at about 30 % of the rated current at a low bus voltage with a
   current-limited supply; a failure names the wiring or the switch to look at.
4. Lower `i_trip_ma` and `vbus_max_mv` once to see each trip and the fault path work.
