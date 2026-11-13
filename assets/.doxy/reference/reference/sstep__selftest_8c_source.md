

# File sstep\_selftest.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_selftest.c**](sstep__selftest_8c.md)

[Go to the documentation of this file](sstep__selftest_8c.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Self-test of the power stage and the motor wiring, one step per fast loop period.
 *
 * Author:      Anton Vynohradov
 * Email:       avynohradov@systemfromscratch.com
 *
 * License:     Apache License 2.0
 *
 * Copyright (c) 2026 Anton Vynohradov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/


/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "sstep_internal.h"

#if (SSTEP_CFG_SELFTEST == 1) /* the whole file */

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

#define BOOT_MS 2U
#define RAMP_MS 20U
#define HOLD_MS 2U
#define DECAY_MS 5U

#define SETTLE_MS 100U

#define PH_BOOT 0U
#define PH_RAMP 1U
#define PH_HOLD 2U
#define PH_DECAY 3U
#define PH_DONE 4U

/* ==============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static int32_t iabs(int32_t x);
static uint32_t periods(uint32_t ms, uint32_t hz);
static void next_step(struct sstep_selftest_ctx* st, uint8_t phase);
static int32_t settle(struct sstep_selftest_ctx* st, bool ok, uint16_t window, uint32_t ms);
static enum sstep_selftest_result drive_step(struct sstep_selftest_ctx* st, int32_t own,
                                             int32_t other);
static enum sstep_selftest_result decay_step(struct sstep_selftest_ctx* st, int32_t i_a,
                                             int32_t i_b, int32_t quarter);

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

static int32_t iabs(int32_t x)
{
    return (x < 0) ? -x : x;
}

static uint32_t periods(uint32_t ms, uint32_t hz)
{
    uint32_t n = (uint32_t)(((uint64_t)ms * hz) / 1000U);
    return (n == 0U) ? 1U : n;
}

static void next_step(struct sstep_selftest_ctx* st, uint8_t phase)
{
    st->phase = phase;
    st->n = 0U;
    st->clean = 0U;
    st->windows = 0U;
}

static int32_t settle(struct sstep_selftest_ctx* st, bool ok, uint16_t window, uint32_t ms)
{
    int32_t r = 0;

    st->clean = ok ? (uint16_t)(st->clean + 1U) : 0U; /* < window <= 50000 */
    st->n++;
    if (st->clean >= window)
    {
        r = 1;
    }
    else if (st->n >= window)
    {
        st->n = 0U;
        st->windows++; /* <= SETTLE_MS / HOLD_MS = 50 */
        if (((uint32_t)st->windows * ms) >= SETTLE_MS)
        {
            r = -1;
        }
    }
    else
    {
        /* go on */
    }
    return r;
}

static enum sstep_selftest_result decay_step(struct sstep_selftest_ctx* st, int32_t i_a,
                                             int32_t i_b, int32_t quarter)
{
    enum sstep_selftest_result r = SSTEP_SELFTEST_RUNNING;
    bool quiet = (iabs(i_a) <= quarter) && (iabs(i_b) <= quarter);
    int32_t s = settle(st, quiet, st->decay_periods, DECAY_MS);

    if (s < 0)
    {
        r = SSTEP_SELFTEST_LEAK; /* current with every leg low that does not die out */
    }
    else if (s == 0)
    {
        /* decaying */
    }
    else if (st->coil != 0U)
    {
        r = SSTEP_SELFTEST_PASS;
    }
    else
    {
        st->coil = 1U;
        st->duty_q8 = 0U;
        next_step(st, PH_RAMP);
    }
    return r;
}

static enum sstep_selftest_result drive_step(struct sstep_selftest_ctx* st, int32_t own,
                                             int32_t other)
{
    enum sstep_selftest_result r = SSTEP_SELFTEST_RUNNING;

    if (st->phase == PH_RAMP)
    {
        if (own >= st->i_test)
        {
            if (st->duty_q8 < st->duty_short_q8)
            {
                r = SSTEP_SELFTEST_SHORT;
            }
            else
            {
                next_step(st, PH_HOLD);
            }
        }
        else if (-own >= st->i_test)
        {
            r = SSTEP_SELFTEST_POLARITY;
        }
        else
        {
            st->duty_q8 += st->ramp_q8;
            if (st->duty_q8 > st->duty_max_q8)
            {
                r = SSTEP_SELFTEST_OPEN;
            }
        }
    }
    else
    {
        /* Wrong wiring returns the whole test current through the other phase, for good; a
           rotor moving into the step induces a current there only until it is at rest. */
        int32_t s = settle(st, (2 * iabs(other)) <= iabs(own), st->hold_periods, HOLD_MS);

        if (s > 0)
        {
            st->i_own = own;
            next_step(st, PH_DECAY);
        }
        else if (s < 0)
        {
            r = SSTEP_SELFTEST_WRONG_PHASE;
        }
        else
        {
            /* holding */
        }
    }
    return r;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS (library internal)
 * =============================================================================*/

void sstep_selftest_setup(struct sstep_selftest_ctx* st, const struct sstep_config* cfg,
                          uint16_t vbus_raw, uint32_t i_test_ma)
{
    const struct sstep_stage_cfg* s = &cfg->stage;
    uint64_t span = (uint64_t)((s->adc_i_span < 0) ? -s->adc_i_span : s->adc_i_span);
    uint64_t period = s->pwm_period_ticks;
    uint64_t usable = period - s->sample_window_ticks;
    uint64_t vbus_mv = ((uint64_t)vbus_raw * s->adc_vbus_mv_full) / s->adc_full;
    uint64_t v_test_mv = ((uint64_t)i_test_ma * cfg->motor.r_mohm) / 1000U;
    uint64_t loss = (2U * (uint64_t)s->dead_time_ns * s->pwm_freq_hz * period) / 1000000000U;
    uint64_t i_test = ((uint64_t)i_test_ma * span) / s->i_full_scale_ma;
    uint64_t duty_max;
    uint64_t duty_short;
    uint32_t ramp;

    if (vbus_mv == 0U)
    {
        vbus_mv = 1U;
    }
    if (i_test == 0U)
    {
        i_test = 1U;
    }
    duty_max = ((3U * v_test_mv * period) / vbus_mv) + loss;
    if (duty_max > usable)
    {
        duty_max = usable;
    }
    if (duty_max == 0U)
    {
        duty_max = 1U;
    }
    duty_short = ((v_test_mv * period) / (4U * vbus_mv)) + loss;
    if (duty_short > duty_max)
    {
        duty_short = duty_max;
    }

    st->i_test = (int32_t)i_test; /* <= span <= 65535 */
    st->duty_max_q8 = (uint32_t)(duty_max * 256U);
    st->duty_short_q8 = (uint32_t)(duty_short * 256U);
    ramp = st->duty_max_q8 / periods(RAMP_MS, cfg->ctrl.fast_loop_hz);
    st->ramp_q8 = (ramp == 0U) ? 1U : ramp;
    /* at most 5 ms: <= 50000 periods */
    st->boot_periods = (uint16_t)periods(BOOT_MS, cfg->ctrl.fast_loop_hz);
    st->hold_periods = (uint16_t)periods(HOLD_MS, cfg->ctrl.fast_loop_hz);
    st->decay_periods = (uint16_t)periods(DECAY_MS, cfg->ctrl.fast_loop_hz);
    st->leg[0] = 0U;
    st->leg[1] = (s->topology == SSTEP_TOPOLOGY_FULL_BRIDGE) ? 2U : 1U;
    st->coil = 0U;
    st->over = 0U;
    st->over_windows = 0U;
    st->duty_q8 = 0U;
    next_step(st, PH_BOOT);
    st->i_own = 0;
    st->result = SSTEP_SELFTEST_RUNNING;
    st->done = 0U;
}

enum sstep_selftest_result sstep_selftest_step(struct sstep_selftest_ctx* st, int32_t i_a,
                                               int32_t i_b, uint16_t compare[SSTEP_LEGS_MAX])
{
    enum sstep_selftest_result r = st->result;

    for (uint32_t k = 0U; k < SSTEP_LEGS_MAX; k++)
    {
        compare[k] = 0U;
    }

    if (st->phase != PH_DONE)
    {
        int32_t own = (st->coil == 0U) ? i_a : i_b;
        int32_t other = (st->coil == 0U) ? i_b : i_a;
        int32_t quarter = (st->i_test < 4) ? 1 : (st->i_test / 4);

        /* Above twice the test current without a break for SETTLE_MS: a short. A rotor
           swinging into the step induces currents that cross zero every half swing; a hard
           short trips the overcurrent protection of the fast loop at once. */
        bool over = (iabs(i_a) > (2 * st->i_test)) || (iabs(i_b) > (2 * st->i_test));

        st->over = over ? (uint16_t)(st->over + 1U) : 0U;
        if (!over)
        {
            st->over_windows = 0U;
        }
        else if (st->over >= st->hold_periods)
        {
            st->over = 0U;
            st->over_windows++; /* <= SETTLE_MS / HOLD_MS = 50 */
        }
        else
        {
            /* above, within a window */
        }

        if (((uint32_t)st->over_windows * HOLD_MS) >= SETTLE_MS)
        {
            r = SSTEP_SELFTEST_SHORT;
        }
        else if (st->phase == PH_BOOT)
        {
            st->n++;
            if (st->n >= st->boot_periods)
            {
                if ((iabs(i_a) > quarter) || (iabs(i_b) > quarter))
                {
                    r = SSTEP_SELFTEST_LEAK;
                }
                else
                {
                    next_step(st, PH_RAMP);
                }
            }
        }
        else if (st->phase == PH_DECAY)
        {
            r = decay_step(st, i_a, i_b, quarter);
        }
        else
        {
            r = drive_step(st, own, other);
        }

        if (r != SSTEP_SELFTEST_RUNNING)
        {
            if (r != SSTEP_SELFTEST_PASS)
            {
                st->i_own = own;
            }
            st->result = r;
            st->phase = PH_DONE;
        }
        else if ((st->phase == PH_RAMP) || (st->phase == PH_HOLD))
        {
            compare[st->leg[st->coil]] = (uint16_t)(st->duty_q8 / 256U);
        }
        else
        {
            /* every leg low */
        }
    }
    return r;
}

#endif /* SSTEP_CFG_SELFTEST */
```


