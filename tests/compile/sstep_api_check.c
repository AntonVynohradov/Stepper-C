/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Compile check of the public API: example configuration (17HS19-2004S1 on
 *              B-G431B-ESC1) and one call of each function. Compiled, not linked.
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


#include "sstep.h"


/* The public API must not expose the library internals */
#if defined(SSTEP_MATH_H) || defined(SSTEP_INTERNAL_H) || defined(SSTEP_SIN_TABLE_H)
#error "sstep.h includes a private header"
#endif

static int pwm(void* c, const uint16_t cmp[SSTEP_LEGS_MAX]) { (void)c; (void)cmp; return 0; }
static int en(void* c, bool e) { (void)c; (void)e; return 0; }

/* 17HS19-2004S1 on B-G431B-ESC1 (ADC scaling values are placeholders until measured) */
const struct sstep_config cfg_17hs19_esc1 = {
    .motor = { .r_mohm = 1600U, .l_uh = 3000U, .i_rated_ma = 2000U,
               .ke_uv_s_per_rad = 0U, .full_steps_per_rev = 200U },
    .stage = { .topology = SSTEP_TOPOLOGY_SHARED_LEG, .vbus_nominal_mv = 24000U,
               .vbus_min_mv = 10000U, .vbus_max_mv = 28000U, .i_full_scale_ma = 4000U,
               .i_trip_ma = 3500U, .pwm_period_ticks = 4250U, .pwm_freq_hz = 20000U,
               .dead_time_ns = 800U, .adc_i_zero = 2048U, .adc_i_span = 2047,
               .adc_vbus_mv_full = 34320U, .adc_full = 4095U },
    .ctrl = { .drive_mode = SSTEP_DRIVE_CURRENT, .fast_loop_hz = 20000U, .slow_loop_hz = 1000U,
              .microsteps = 16U, .current_bw_hz = 1000U, .i_run_ma = 1500U, .i_hold_ma = 500U,
              .hold_delay_ms = 500U, .v_max = 16000U, .accel = 64000U, .decel = 0U },
    .hal = { .context = NULL, .pwm_write = pwm, .output_enable = en, .on_fault = NULL, .on_event = NULL },
};

int use_api(void);
int use_api(void)
{
    static struct sstep m;
    struct sstep_sample s = { 2048U, 2048U, 2860U };
    struct sstep_status st;
    int r = sstep_init(&m, &cfg_17hs19_esc1);
    r |= sstep_enable(&m);
    r |= sstep_fast_loop(&m, &s);
    r |= sstep_slow_loop(&m);
    r |= sstep_move_to(&m, 3200);
    r |= sstep_get_status(&m, &st);
    return r;
}
