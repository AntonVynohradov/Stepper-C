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

/** Durations of the steps [ms]. */
#define BOOT_MS 2U
#define RAMP_MS 20U
#define HOLD_MS 2U
#define DECAY_MS 5U

/** Steps of the test (sstep_selftest::phase). */
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
static enum sstep_selftest_result drive_step(struct sstep_selftest_ctx* st, int32_t own,
                                             int32_t other);

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

/** @brief |x| for currents in ADC counts (|x| <= 65535). */
static int32_t iabs(int32_t x)
{
    return (x < 0) ? -x : x;
}

/** @brief Fast loop periods of ms milliseconds, at least one (<= 200000: 20 ms at the
 *         largest fast loop rate of 10 MHz). */
static uint32_t periods(uint32_t ms, uint32_t hz)
{
    uint32_t n = (uint32_t)(((uint64_t)ms * hz) / 1000U);
    return (n == 0U) ? 1U : n;
}

/** @brief Ramp and hold of one phase. */
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
                st->phase = PH_HOLD;
                st->n = 0U;
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
        st->n++;
        if (st->n >= st->hold_periods)
        {
            /* Wrong wiring returns the whole test current through the other phase; a rotor
               moving into the step induces a smaller current there. */
            if ((2 * iabs(other)) > iabs(own))
            {
                r = SSTEP_SELFTEST_WRONG_PHASE;
            }
            else
            {
                st->i_own = own;
                st->phase = PH_DECAY;
                st->n = 0U;
            }
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
    st->phase = PH_BOOT;
    st->coil = 0U;
    st->n = 0U;
    st->duty_q8 = 0U;
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

        if ((iabs(i_a) > (2 * st->i_test)) || (iabs(i_b) > (2 * st->i_test)))
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
                    st->phase = PH_RAMP;
                    st->n = 0U;
                }
            }
        }
        else if (st->phase == PH_DECAY)
        {
            st->n++;
            if (st->n >= st->decay_periods)
            {
                if (st->coil != 0U)
                {
                    r = SSTEP_SELFTEST_PASS;
                }
                else
                {
                    st->coil = 1U;
                    st->phase = PH_RAMP;
                    st->n = 0U;
                    st->duty_q8 = 0U;
                }
            }
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
