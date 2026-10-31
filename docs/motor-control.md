# Motor Control

This chapter describes the algorithms of Stepper-C and the equations behind them: the
motor model, per-unit scaling, the current controller, microstepping, the motion
profile, the voltage limits of the two power stage topologies and the numeric backends.
Every number in this chapter comes from the code or from the closed-loop tests on the
motor model (`tests/unit/gtest_sstep_loop.cpp`).

## Motor model

A two-phase hybrid stepper motor with $N_s$ full steps per revolution has $p = N_s / 4$
pole pairs (50 for a 1.8° motor). With the rotor angle $\theta$ and its speed $\omega$, each phase is
a resistance, an inductance and a back-EMF:

$$
\begin{aligned}
v_A &= R\,i_A + L\,\frac{di_A}{dt} - K_t\,\omega \sin(p\theta) \\
v_B &= R\,i_B + L\,\frac{di_B}{dt} + K_t\,\omega \cos(p\theta) \\
T_e &= K_t \left( i_B \cos(p\theta) - i_A \sin(p\theta) \right)
\end{aligned}
$$

$K_t$ is the torque constant [N·m/A]; in SI units it equals the back-EMF constant
[V·s/rad], the value entered as `ke_uv_s_per_rad`.

The library imposes the phase currents

$$
i_A = I \cos\varphi \qquad i_B = I \sin\varphi
$$

where $\varphi$ is the electrical angle commanded by the trajectory. The torque becomes

$$
T_e = K_t\, I \sin(\varphi - p\theta)
$$

The rotor follows $\varphi$ with a lag (the *load angle*) that grows
with the load. The motor holds up to $K_t I$ (0.42 N·m for 2 A with the 17HS19-2004S1)
at a load angle of 90° and loses steps above it.

The closed-loop test with a 0.25 N·m load at 1 rev/s measures a load angle of 37.21°,
against $\arcsin(0.25 / 0.4172) = 36.83°$ predicted by the equation; with 0.50 N·m the
motor slips, as expected.

## Per-unit scaling

Internally every signal is a fraction of a base value, so that the same arithmetic
works for a 0.5 A and a 10 A motor:

| Quantity | Base | Range inside the loops |
| --- | --- | --- |
| Current | `i_full_scale_ma`: the current at ADC full scale | -1 .. +1 pu |
| Voltage | `vbus_nominal_mv`: the nominal bus voltage | -1 .. +1 pu for signals; up to ±65 536 pu in accumulators |
| Bus voltage | `vbus_nominal_mv` | 0 .. 2 pu |
| Angle | One electrical turn = $2^{32}$ | `uint32_t`, wraps |

`sstep_init()` converts every engineering value of the configuration into per-unit
gains with integer arithmetic ($2\pi \approx 710 / 113$, error $8.5 \cdot 10^{-8}$), so a fixed-point
build needs no floating point at all.

## Microstepping

![Current vector and microsteps](assets/images/microstep.svg)

One full step is a quarter of an electrical turn ($2^{30}$). With `microsteps` $= M$, one
microstep is $2^{30} / M$ of the 32-bit angle; positions are kept in Q16 microsteps
(1/65 536 microstep) in 64 bits, so the angle is exact and never drifts.

The sine and cosine come from a quarter-wave table with linear interpolation
(`SSTEP_CFG_SINE_TABLE_BITS`, default $2^8$ intervals, 257 entries in flash). Measured
worst-case error over a full turn:

| Table bits | Fixed point | Float |
| --- | --- | --- |
| 6 | $1.08 \cdot 10^{-4}$ (3.5 LSB of Q15) | $7.5 \cdot 10^{-5}$ |
| 8 (default) | $3.1 \cdot 10^{-5}$ (1.0 LSB of Q15) | $4.8 \cdot 10^{-6}$ |
| 10 | $3.1 \cdot 10^{-5}$ (1.0 LSB of Q15) | $3.5 \cdot 10^{-7}$ |

With the fixed-point backend the default table is already at the resolution of Q15;
a larger table only helps the float backend.

Between two slow-loop periods the fast loop advances the angle by a constant increment,
so the current vector turns smoothly at the PWM rate instead of jumping once per
millisecond.

## Current control

![Current control loop](assets/images/control-loop.svg)

Each phase has its own PI controller. The reference comes from the angle and the
amplitude, the measurement from the ADC:

$$
e = i_\mathrm{ref} - i_\mathrm{meas} \qquad
v = K_p\, e + K_i \int e\, dt + v_\mathrm{ff}
$$

**Gains.** `sstep_init()` places the zero of the controller on the pole of the winding
($R / L$), which leaves an integrator with the crossover frequency
$\omega_c = 2\pi \cdot$ `current_bw_hz`:

$$
K_p = L\,\omega_c \qquad K_i = R\,\omega_c \qquad\Longrightarrow\qquad
\frac{i_\mathrm{meas}}{i_\mathrm{ref}} = \frac{\omega_c}{s + \omega_c}
$$

The closed loop is a first-order low-pass with the requested bandwidth; no manual
tuning is needed. The bandwidth must not exceed `fast_loop_hz / 4` (checked by
`sstep_init()`).

**Back-EMF feed-forward.** At speed the motor generates a voltage that the PI would
otherwise have to compensate with an error. The library adds it in advance from the
commanded speed:

$$
v_{A,\mathrm{ff}} = -K_e\,\omega \sin\varphi \qquad
v_{B,\mathrm{ff}} = K_e\,\omega \cos\varphi \qquad
\omega = \frac{2\pi\, v}{N_s M}
$$

with $v$ in microsteps/s. With `ke_uv_s_per_rad = 0` the feed-forward is off.

**Anti-windup.** When the requested voltage exceeds what the power stage can apply
(see [Voltage limits](#voltage-limits)), the integrator is corrected by the difference
between the applied and the requested voltage (back-calculation with the tracking time
constant $T_t = L / R$, gain $T_s R / L$). The integrator does not wind up while the voltage
is limited.

**Measured response** (17HS19-2004S1, 24 V, 20 kHz, 1 kHz bandwidth, locked rotor, step
0 → 2 A, ADC 12 bits):

| Topology | Overshoot | Rise time 0–90 % | Settling ±2 % | Final value |
| --- | --- | --- | --- | --- |
| Full bridge | 1.11 % | 0.350 ms | 0.400 ms | 2.0002 A |
| Shared leg | 1.11 % | 0.350 ms | 0.400 ms | 1.9999 A |

The float backend gives the same numbers within 0.01 %. A first-order system with
1 kHz bandwidth rises to 90 % in $2.3 / \omega_c = 0.37$ ms; the small overshoot comes from the
one-period delay between measurement and PWM update.

### Voltage drive

Boards without current sensing use `SSTEP_DRIVE_VOLTAGE`: the controller is replaced
by $v = R\, i_\mathrm{ref} + v_\mathrm{ff}$. The current settles at the reference at standstill (test:
2.0 A within 10 mA) but the torque drops at speed, where the inductance and the
error of the back-EMF estimate are not corrected. The overcurrent trip is not active in
this mode (there is no current measurement).

## Bus voltage compensation

The controllers compute voltages in per unit of the *nominal* bus voltage. The fast loop
converts them to duty cycles with the *measured* bus voltage ($V_\mathrm{base} / V_\mathrm{bus}$, updated
every slow period), so the phase voltage, and with it the current, does not depend on
the supply. Test: with the supply at half its nominal value, the current still settles
at 2.0 A within 4 mA.

## Voltage limits

![Power stage topologies](assets/images/topologies.svg)

The two supported topologies differ in how much phase voltage they can apply:

| Topology | Legs | Phase voltages | Limit (relative to Vbus) | Sinusoidal amplitude |
| --- | --- | --- | --- | --- |
| `SSTEP_TOPOLOGY_FULL_BRIDGE` | 4: A+, A−, B+, B− | $v_A = A^+ - A^-$, $v_B = B^+ - B^-$ | $\max(\lvert v_A \rvert, \lvert v_B \rvert) \le 1$ | $V_\mathrm{bus}$ |
| `SSTEP_TOPOLOGY_SHARED_LEG` | 3: U, V, W (shared) | $v_A = U - W$, $v_B = V - W$ | $\max(v_A, v_B, 0) - \min(v_A, v_B, 0) \le 1$ | about $V_\mathrm{bus} / \sqrt{2}$ |

When the request is beyond the limit, both phase voltages are scaled down by the same
factor: the direction of the voltage vector is kept, only its length is reduced, and the
anti-windup takes the difference into account.

**Duty cycles.** Full bridge: each phase is centred at 50 %. Shared leg: the shared leg
W is placed in the middle of the available range:

$$
A^{\pm} = \frac{1}{2} \pm \frac{v_A}{2}, \quad B^{\pm} = \frac{1}{2} \pm \frac{v_B}{2}
\qquad\qquad
W = \frac{1}{2} - \frac{\max(v_A, v_B, 0) + \min(v_A, v_B, 0)}{2}, \quad
U = v_A + W, \quad V = v_B + W
$$

The fourth compare value is 0 for the shared leg.

**Consequence at speed.** At 8 rev/s (2 A requested, 24 V) the measured current
amplitude is 1.862 A with a full bridge and 1.557 A with a shared leg: the three-leg
stage reaches its voltage limit first. Choose the supply voltage for the highest speed
of the application.

## Motion profile

![Trapezoidal motion profile](assets/images/trajectory.svg)

The slow loop advances an integer trajectory once per period; positions are Q16
microsteps (64 bits), velocities Q16 microsteps per period, so a position is exact after
any number of moves.

**Move (`sstep_move_to()`, `sstep_move_by()`).** In each period the planner chooses the
largest velocity from which it can still stop exactly on the target with the
deceleration limit:

$$
v_\mathrm{new} = \frac{-d + \sqrt{d^2 + 8\,d\,\lvert r \rvert - 4\,d\,\lvert v \rvert}}{2}
$$

with the deceleration $d$ and the remaining distance $r$, both per slow period,

then capped at `v_max` and changed by at most `accel` (speeding up) or `decel` (slowing down)
per period. The result is a trapezoid (or a triangle for short moves) that ends on the
target with zero velocity, without overshoot. A new target during a move is followed
smoothly: there is no velocity step, and a target behind the motor makes it decelerate,
reverse and approach from the other side.

**Run (`sstep_run()`).** Accelerate or decelerate with the limits to the requested
signed velocity and keep it. A reversal passes through zero with `decel` and then
`accel`.

**Stop (`sstep_stop()`).** Decelerate to zero with `decel`; the target becomes the
position where the motor stops.

**Measured** (16 microsteps, `v_max` 16 000 microsteps/s, `accel` 160 000
microsteps/s²): a move of 6 400 microsteps (2 revolutions) ends after 499 ms with the
rotor at 6400.00 microsteps and a largest load angle of 10.8°; a reversal from +4 to
−4 rev/s stays below a load angle of 8.7°.

**Resolution.** With `slow_loop_hz = 1000` the velocity resolution is
$1000 / 65536 = 0.015$ microsteps/s. `sstep_set_motion_limits()` rejects values that
round to zero or reach $2^{30}$ Q16 per period.

## Hold current

After `hold_delay_ms` without motion the current amplitude drops from `i_run_ma` to
`i_hold_ma` and `on_event(SSTEP_EVENT_HOLD_CURRENT)` is reported. Any motion switches
back to the run current in the same period. The hold current reduces heating at
standstill; the holding torque is $K_t\, i_\mathrm{hold}$.

## Numeric backends

| | Fixed point | Float |
| --- | --- | --- |
| Select | `SSTEP_MATH_BACKEND=FIXED` (default) | `SSTEP_MATH_BACKEND=FLOAT` |
| Signals | Q15 in `int16_t` | `float` |
| Accumulators | Q15 in `int32_t` (±65 536) | `float`, clamped to the same range |
| Integrators | Q27 in `int32_t` (±16 pu) | `float`, clamped to ±16 |
| Gains | Q15 mantissa and power-of-two exponent (−16 .. 15) | `float` |
| Arithmetic | Saturating; rounding half up; no undefined shifts | Saturating to the same ranges |
| Typical target | Cortex-M0/M0+/M3, RISC-V without F extension | Cortex-M4F/M7/M33 with FPU, STM32G4 |

Both backends saturate at the same limits, so the control behaves the same; the unit
tests and the closed-loop tests run against both. The motion planner and the
configuration are integer code in both backends.
