---
title: Stepper-C — Stepper Motor Control in C
description: Closed-loop microstepping current control for two-phase stepper motors, in portable C11 for any MCU, with no heap, no RTOS and MISRA C:2012.
template: home.html
hide:
  - navigation
  - toc
---

<span class="sfs-eyebrow">Quick start</span>

## A motor running in one file

Two callbacks connect the library to the power stage: one writes the PWM compare values,
one switches the gate drivers. The PWM/ADC interrupt passes the raw ADC readings to the
fast loop; the main loop runs the slow loop once per millisecond and gives the commands.

```c
#include "sstep.h"

static struct sstep motor;

static int pwm_write(void* context, const uint16_t compare[SSTEP_LEGS_MAX])
{
    (void)context;
    board_pwm_set(compare);
    return 0;
}

static int output_enable(void* context, bool enable)
{
    (void)context;
    board_gates_enable(enable);
    return 0;
}

/* 17HS19-2004S1 on a B-G431B-ESC1: 24 V, 20 kHz PWM, 16 microsteps */
static const struct sstep_config motor_config = {
    .motor = {.r_mohm = 1600U, .l_uh = 3000U, .i_rated_ma = 2000U,
              .ke_uv_s_per_rad = 208600U, .full_steps_per_rev = 200U},
    .stage = {.topology = SSTEP_TOPOLOGY_SHARED_LEG, .vbus_nominal_mv = 24000U,
              .vbus_min_mv = 10000U, .vbus_max_mv = 28000U, .i_full_scale_ma = 4000U,
              .i_trip_ma = 3500U, .pwm_period_ticks = 4250U, .pwm_freq_hz = 20000U,
              .adc_i_zero = 2048U, .adc_i_span = 2047, .adc_vbus_mv_full = 34320U,
              .adc_full = 4095U},
    .ctrl = {.drive_mode = SSTEP_DRIVE_CURRENT, .fast_loop_hz = 20000U,
             .slow_loop_hz = 1000U, .microsteps = 16U, .current_bw_hz = 1000U,
             .i_run_ma = 1500U, .i_hold_ma = 500U, .hold_delay_ms = 500U,
             .v_max = 16000U, .accel = 64000U},
    .hal = {.pwm_write = pwm_write, .output_enable = output_enable},
};

/* PWM/ADC interrupt, 20 kHz */
void ADC1_2_IRQHandler(void);
void ADC1_2_IRQHandler(void)
{
    const struct sstep_sample sample = {board_adc_phase_a(), board_adc_phase_b(),
                                        board_adc_vbus()};
    (void)sstep_fast_loop(&motor, &sample);
}

int main(void)
{
    board_init();
    if ((sstep_init(&motor, &motor_config) != SSTEP_OK) || (sstep_enable(&motor) != SSTEP_OK))
    {
        for (;;)
        {
        }
    }
    (void)sstep_move_to(&motor, 3200); /* one revolution */
    for (;;)
    {
        if (board_tick_1ms())
        {
            (void)sstep_slow_loop(&motor);
        }
    }
}
```

The `board_…` functions stand for your timer, ADC and GPIO code; the ADC scaling values
above must be measured on your board ([Configuration](configuration.md#power-stage-and-measurement)).

## Find your way

| If you want to… | Read |
| --- | --- |
| Understand what the library does and does not do | [Introduction](overview.md) |
| Build it, run the tests, add it to your project | [Getting Started](guide.md) |
| Understand the loops, states and memory | [How It Works](architecture.md) |
| Configure a motor and command motion | [Using the Library](usage.md) |
| Understand the current control, microstepping and limits | [Motor Control](motor-control.md) |
| Connect it to timers, ADC and interrupts | [Integration Guide](integration.md) |
| Look up a function | [API Reference](api.md) |
| Read everything offline or print it | [PDF user manual](https://antonvynohradov.github.io/Stepper-C/downloads/Stepper-C_User_Manual.pdf) |
