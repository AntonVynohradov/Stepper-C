/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public API: initialisation, fast loop (current control), slow loop (motion), commands.
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

#include "sstep.h"

#include <stddef.h>

#include "sstep_internal.h"

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** 2 pi = 710 / 113 (error 8.5e-8). */
#define TWO_PI_NUM 710U
#define TWO_PI_DEN 113U

/** Operand limit of the gain ratios (sstep_gain_from_ratio64). */
#define RATIO_LIMIT ((uint64_t)1U << 62U)

/** Largest engineering value accepted in the motor and stage configuration. */
#define VALUE_MAX 10000000U

/** Velocities and accelerations in Q16 per period stay below 2^30. */
#define Q16_LIMIT ((int64_t)0x40000000)

/** Bus voltage limit of the fast loop: 2 pu - 2^-15. */
#define VBUS_PU_MAX SSTEP_ACC_SCALE_MAX

/* ==============================================================================
 * PRIVATE TYPE DEFINITIONS
 * =============================================================================*/

/* Alignment probes: sizeof(probe) - sizeof(x) is the alignment of the type of x
   (no _Alignof, Rule 1.4; no offsetof, which the static analysis cannot resolve) */
struct align_ctx
{
    char c;
    struct sstep_ctx x;
};

struct align_pub
{
    char c;
    struct sstep x;
};

/* The opaque public instance must hold the private driver state */
_Static_assert(sizeof(struct sstep_config) <= SSTEP_CONFIG_BYTES,
               "SSTEP_CONFIG_BYTES in sstep_ctrl.h is too small for this ABI");
_Static_assert(sizeof(struct sstep_ctx) <= sizeof(struct sstep),
               "SSTEP_STATE_BYTES in sstep_ctrl.h is too small");
_Static_assert((sizeof(struct align_ctx) - sizeof(struct sstep_ctx)) <=
                   (sizeof(struct align_pub) - sizeof(struct sstep)),
               "struct sstep is not aligned for struct sstep_ctx");

/* ==============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static bool is_pow2(uint32_t v);
static uint8_t log2_u32(uint32_t v);
static bool mul_ok(uint64_t a, uint64_t b, uint64_t* out);
static int ratio(uint64_t n1, uint64_t n2, uint64_t d1, uint64_t d2, struct sstep_gain* g);
static int64_t div_round(int64_t num, int64_t den);
static int64_t pow2_i64(uint32_t n);
static sstep_real_t gain_apply_i32(const struct sstep_gain* g, int32_t v);
static sstep_angle_t raw_angle(const struct sstep_ctx* m, int64_t pos);
static int check_config(const struct sstep_config* cfg);
static int check_stall(const struct sstep_config* cfg);
static int derive_stall(struct sstep_ctx* m);
static int set_currents(struct sstep_ctx* m, uint32_t run_ma, uint32_t hold_ma);
static int set_limits(struct sstep_ctx* m, uint32_t v_max, uint32_t accel, uint32_t decel);
static int derive(struct sstep_ctx* m);
static void shutdown(struct sstep_ctx* m, uint16_t faults);
static bool vbus_out_of_range(const struct sstep_ctx* m, bool need_sample);
#if (SSTEP_CFG_SELFTEST == 1)
static bool selftest_running(const struct sstep_ctx* m);
static void report_selftest(struct sstep_ctx* m);
static void selftest_period(struct sstep_ctx* m, int32_t da, int32_t db);
#endif
static void derive_window(struct sstep_ctx* m);
static void derive_slew(struct sstep_ctx* m);
static sstep_real_t slew_current(const struct sstep_ctx* m, sstep_real_t target);
static void publish(struct sstep_ctx* m, const struct sstep_setpoint* sp);
static bool is_active(enum sstep_state s);
static bool is_moving(const struct sstep_ctx* m);
static void control(struct sstep_ctx* m);

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

/** @brief v is a power of two (v > 0). */
static bool is_pow2(uint32_t v)
{
    return (v != 0U) && ((v & (v - 1U)) == 0U);
}

/** @brief log2 of a power of two. */
static uint8_t log2_u32(uint32_t v)
{
    uint8_t n = 0U;
    uint32_t x = v;
    while (x > 1U)
    {
        x >>= 1U;
        n++;
    }
    return n;
}

/** @brief out = a * b; false when the product reaches 2^62. */
static bool mul_ok(uint64_t a, uint64_t b, uint64_t* out)
{
    bool ok = (a == 0U) || (b < (RATIO_LIMIT / a));
    *out = ok ? (a * b) : 0U;
    return ok;
}

/** @brief g = (n1 n2) / (d1 d2) with overflow checks (configuration only). Every factor
 *         is a product of at most two validated values (< 10^7 each), so it fits. */
static int ratio(uint64_t n1, uint64_t n2, uint64_t d1, uint64_t d2, struct sstep_gain* g)
{
    int result = SSTEP_ERR_CONFIG;
    uint64_t n;
    uint64_t d;
    if (mul_ok(n1, n2, &n) && mul_ok(d1, d2, &d) &&
        (sstep_gain_from_ratio64((int64_t)n, (int64_t)d, g) == SSTEP_OK))
    {
        result = SSTEP_OK;
    }
    return result;
}

/** @brief round(num / den) for den > 0, half away from zero. */
static int64_t div_round(int64_t num, int64_t den)
{
    return (num >= 0) ? ((num + (den / 2)) / den) : -((-num + (den / 2)) / den);
}

/** @brief 2^n for n = 0 .. 62, built with an unsigned shift (MISRA C:2012 Rule 10.1). */
static int64_t pow2_i64(uint32_t n)
{
    uint64_t p = (uint64_t)1U << n;
    return (int64_t)p;
}

/** @brief g * v for a 32-bit integer v (slow loop, 64-bit product), as a saturated signal. */
static sstep_real_t gain_apply_i32(const struct sstep_gain* g, int32_t v)
{
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
    int64_t p = (int64_t)v * (int64_t)g->mant; /* value * 2^15 = p * 2^exp */
    int32_t e = (int32_t)g->exp;
    int64_t q;
    if (e >= 0)
    {
        q = p * pow2_i64((uint32_t)e); /* |p| < 2^46, exp <= 15 */
    }
    else
    {
        e = -e;
        q = div_round(p, pow2_i64((uint32_t)e));
    }
    if (q > (int64_t)INT16_MAX)
    {
        q = (int64_t)INT16_MAX;
    }
    else if (q < (int64_t)INT16_MIN)
    {
        q = (int64_t)INT16_MIN;
    }
    else
    {
        /* in range */
    }
    return (sstep_real_t)q;
#else
    return sstep_sat_f(g->value * (float)v);
#endif
}

/** @brief Electrical angle of a position [Q16 microsteps]: microstep = 2^30 / M. */
static sstep_angle_t raw_angle(const struct sstep_ctx* m, int64_t pos)
{
    return (sstep_angle_t)(((uint64_t)pos) << m->angle_shift); /* modulo one turn */
}

/** @brief Validate a configuration field by field and as a whole. */
static int check_config(const struct sstep_config* cfg)
{
    const struct sstep_motor_cfg* mo = &cfg->motor;
    const struct sstep_stage_cfg* st = &cfg->stage;
    const struct sstep_ctrl_cfg* c = &cfg->ctrl;
    int result = SSTEP_OK;

    if ((cfg->hal.pwm_write == NULL) || (cfg->hal.output_enable == NULL) || (mo->r_mohm == 0U) ||
        (mo->r_mohm > VALUE_MAX) || (mo->l_uh == 0U) || (mo->l_uh > VALUE_MAX) ||
        (mo->i_rated_ma == 0U) || (mo->i_rated_ma > VALUE_MAX) ||
        (mo->ke_uv_s_per_rad > VALUE_MAX) || (mo->full_steps_per_rev == 0U) ||
        ((st->topology != SSTEP_TOPOLOGY_FULL_BRIDGE) &&
         (st->topology != SSTEP_TOPOLOGY_SHARED_LEG)) ||
        (st->vbus_nominal_mv == 0U) || (st->vbus_nominal_mv > VALUE_MAX) ||
        (st->vbus_min_mv > VALUE_MAX) || (st->vbus_max_mv > VALUE_MAX) ||
        (st->i_full_scale_ma == 0U) || (st->i_full_scale_ma > VALUE_MAX) || (st->i_trip_ma == 0U) ||
        (st->pwm_period_ticks < 2U) || (st->adc_i_span == 0) || (st->adc_i_span > 65535) ||
        (st->adc_i_span < -65535) || (st->adc_full == 0U) || (st->adc_i_zero > st->adc_full) ||
        (st->adc_vbus_mv_full == 0U) || (st->adc_vbus_mv_full > VALUE_MAX) ||
        ((c->drive_mode != SSTEP_DRIVE_CURRENT) && (c->drive_mode != SSTEP_DRIVE_VOLTAGE)) ||
        (c->fast_loop_hz == 0U) || (c->slow_loop_hz == 0U) || (c->fast_loop_hz > VALUE_MAX) ||
        (!is_pow2(c->microsteps)) || (c->microsteps > SSTEP_CFG_MICROSTEPS_MAX) ||
        (c->current_bw_hz == 0U) || (c->i_run_ma == 0U) || (c->v_max == 0U) || (c->accel == 0U) ||
        (c->i_slew_ma_per_ms > VALUE_MAX))
    {
        result = SSTEP_ERR_PARAM;
    }
#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 0)
    else if (st->topology == SSTEP_TOPOLOGY_FULL_BRIDGE)
    {
        result = SSTEP_ERR_UNSUPPORTED;
    }
#endif
#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 0)
    else if (st->topology == SSTEP_TOPOLOGY_SHARED_LEG)
    {
        result = SSTEP_ERR_UNSUPPORTED;
    }
#endif
    else if ((st->i_trip_ma > st->i_full_scale_ma) || ((c->fast_loop_hz % c->slow_loop_hz) != 0U) ||
             ((c->fast_loop_hz / c->slow_loop_hz) > 65535U) ||
             (c->current_bw_hz > (c->fast_loop_hz / 4U)) ||
             (st->sample_window_ticks > (st->pwm_period_ticks / 4U)) ||
             ((st->vbus_min_mv != 0U) && (st->vbus_min_mv >= st->vbus_nominal_mv)) ||
             ((st->vbus_max_mv != 0U) && (st->vbus_max_mv <= st->vbus_nominal_mv)))
    {
        result = SSTEP_ERR_CONFIG;
    }
    else
    {
        result = check_stall(cfg);
    }
    return result;
}

/** @brief Validate the stall detection fields (ignored when stall_time_ms is 0). */
static int check_stall(const struct sstep_config* cfg)
{
    const struct sstep_ctrl_cfg* c = &cfg->ctrl;
    int result = SSTEP_OK;

    if (c->stall_time_ms == 0U)
    {
        /* stall detection off */
    }
    else if ((c->stall_time_ms > VALUE_MAX) || (c->stall_filter_us == 0U) ||
             (c->stall_filter_us > VALUE_MAX) || (c->stall_speed == 0U) ||
             (c->stall_speed > VALUE_MAX) || (c->stall_sync_pct == 0U) ||
             (c->stall_sync_pct > 100U))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if ((c->drive_mode != SSTEP_DRIVE_CURRENT) || (cfg->motor.ke_uv_s_per_rad == 0U) ||
             (((uint64_t)c->stall_filter_us * c->fast_loop_hz) < 1000000U))
    {
        /* the observer needs measured currents, Ke, and tau >= Ts */
        result = SSTEP_ERR_CONFIG;
    }
    else
    {
        /* consistent */
    }
    return result;
}

/** @brief Derive the stall detection constants (after the other gains). */
static int derive_stall(struct sstep_ctx* m)
{
    const struct sstep_motor_cfg* mo = &m->cfg.motor;
    const struct sstep_stage_cfg* st = &m->cfg.stage;
    const struct sstep_ctrl_cfg* c = &m->cfg.ctrl;
    int result = SSTEP_OK;

    if (c->stall_time_ms != 0U)
    {
        uint64_t ticks = ((uint64_t)c->stall_time_ms * c->slow_loop_hz) / 1000U;
        /* back-EMF of stall_speed: Ke 2 pi v / (Ns M) [uV], every factor below 10^7 */
        uint64_t e_thr_uv = ((uint64_t)c->stall_speed * mo->ke_uv_s_per_rad * TWO_PI_NUM) /
                            ((uint64_t)TWO_PI_DEN * mo->full_steps_per_rev * c->microsteps);
        uint64_t e_thr_mv = (e_thr_uv + 500U) / 1000U;
        /* L / Ts and R / 2 in uOhm: l_uh fs and 500 r_mohm */
        uint64_t l_fs = (uint64_t)mo->l_uh * c->fast_loop_hz;
        uint64_t r_half = (uint64_t)mo->r_mohm * 500U;
        bool b_negative = (l_fs < r_half);

        /* a, b = (L / Ts +/- R / 2) I_base / V_base; Ts / tau = 10^6 / (fs tau_us) <= 1 */
        result =
            ratio(l_fs + r_half, st->i_full_scale_ma, 1000000U, st->vbus_nominal_mv, &m->emf_a);
        if (result == SSTEP_OK)
        {
            result = ratio(b_negative ? (r_half - l_fs) : (l_fs - r_half), st->i_full_scale_ma,
                           1000000U, st->vbus_nominal_mv, &m->emf_b);
            if (b_negative)
            {
                /* R Ts / 2 > L: a very small inductance */
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
                m->emf_b.mant = (int16_t)(-m->emf_b.mant);
#else
                m->emf_b.value = -m->emf_b.value;
#endif
            }
        }
        if (result == SSTEP_OK)
        {
            result = ratio(1000000U, 1U, c->fast_loop_hz, c->stall_filter_us, &m->emf_alpha);
        }
        if ((result == SSTEP_OK) && ((ticks == 0U) || (ticks > UINT32_MAX) || (e_thr_mv == 0U) ||
                                     (e_thr_mv >= st->vbus_nominal_mv)))
        {
            /* shorter than a slow period, or a threshold below 1 mV or above the bus */
            result = SSTEP_ERR_CONFIG;
        }
        if (result == SSTEP_OK)
        {
            m->stall_ticks = (uint32_t)ticks;
            m->stall_emf_mv = (int32_t)e_thr_mv;
        }
    }
    return result;
}

/** @brief Validate and store the run and hold currents. */
static int set_currents(struct sstep_ctx* m, uint32_t run_ma, uint32_t hold_ma)
{
    int result = SSTEP_OK;
    sstep_real_t run;
    sstep_real_t hold;

    if ((run_ma == 0U) || (run_ma > VALUE_MAX) || (hold_ma > VALUE_MAX))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if ((hold_ma > run_ma) || (run_ma > m->cfg.stage.i_trip_ma) ||
             (run_ma > m->cfg.motor.i_rated_ma))
    {
        result = SSTEP_ERR_CONFIG;
    }
    else
    {
        int32_t base = (int32_t)m->cfg.stage.i_full_scale_ma;
        (void)sstep_real_from_ratio((int32_t)run_ma, base, &run); /* base > 0 */
        (void)sstep_real_from_ratio((int32_t)hold_ma, base, &hold);
        m->i_run = run;
        m->i_hold = hold;
        m->cfg.ctrl.i_run_ma = run_ma;
        m->cfg.ctrl.i_hold_ma = hold_ma;
    }
    return result;
}

/** @brief Validate and store the motion limits (engineering units -> Q16 per period). */
static int set_limits(struct sstep_ctx* m, uint32_t v_max, uint32_t accel, uint32_t decel)
{
    int result = SSTEP_OK;
    int64_t f = (int64_t)m->cfg.ctrl.slow_loop_hz;
    uint32_t dec_in = (decel == 0U) ? accel : decel;
    int64_t vq = div_round((int64_t)v_max * SSTEP_POS_ONE, f);
    int64_t aq = div_round((int64_t)accel * SSTEP_POS_ONE, f * f);
    int64_t dq = div_round((int64_t)dec_in * SSTEP_POS_ONE, f * f);

    if ((v_max == 0U) || (accel == 0U) || (vq < 1) || (vq >= Q16_LIMIT) || (aq < 1) ||
        (aq >= Q16_LIMIT) || (dq < 1) || (dq >= Q16_LIMIT))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        m->mo.v_max = (int32_t)vq;
        m->mo.acc = (int32_t)aq;
        m->mo.dec = (int32_t)dq;
        m->mo.brake_vmax = (vq * vq) / (2 * dq);
        m->cfg.ctrl.v_max = v_max;
        m->cfg.ctrl.accel = accel;
        m->cfg.ctrl.decel = decel;
    }
    return result;
}

/** @brief Derive gains, scaling and limits from the (validated) configuration. */
static int derive(struct sstep_ctx* m)
{
    const struct sstep_motor_cfg* mo = &m->cfg.motor;
    const struct sstep_stage_cfg* st = &m->cfg.stage;
    const struct sstep_ctrl_cfg* c = &m->cfg.ctrl;
    uint64_t span = (uint64_t)((st->adc_i_span < 0) ? -st->adc_i_span : st->adc_i_span);
    uint64_t bw2pi = (uint64_t)c->current_bw_hz * TWO_PI_NUM;
    uint64_t vmv = st->vbus_nominal_mv;
    uint64_t trip = ((uint64_t)st->i_trip_ma * span) / st->i_full_scale_ma;
    uint64_t vmax_raw = ((uint64_t)st->vbus_max_mv * st->adc_full) / st->adc_vbus_mv_full;
    uint64_t vmin_raw = (((uint64_t)st->vbus_min_mv * st->adc_full) + st->adc_vbus_mv_full - 1U) /
                        st->adc_vbus_mv_full;
    int result;

    /* Kp = L wc I_base / V_base, Ki Ts = R wc Ts I_base / V_base, Kb = Ts R / L */
    result = ratio((uint64_t)mo->l_uh * st->i_full_scale_ma, bw2pi, (uint64_t)TWO_PI_DEN * 1000000U,
                   vmv, &m->kp);
    if (result == SSTEP_OK)
    {
        result = ratio((uint64_t)mo->r_mohm * st->i_full_scale_ma, bw2pi,
                       (uint64_t)TWO_PI_DEN * 1000U, (uint64_t)c->fast_loop_hz * vmv, &m->ki);
    }
    if (result == SSTEP_OK)
    {
        result = ratio(mo->r_mohm, 1000U, c->fast_loop_hz, mo->l_uh, &m->kb);
    }
    if (result == SSTEP_OK)
    {
        result = ratio(mo->r_mohm, st->i_full_scale_ma, 1000U, vmv, &m->r_pu);
    }
    if (result == SSTEP_OK)
    {
        result = ratio(1U, 1U, span, 1U, &m->adc_i);
        if (st->adc_i_span < 0)
        {
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
            m->adc_i.mant = (int16_t)(-m->adc_i.mant);
#else
            m->adc_i.value = -m->adc_i.value;
#endif
        }
    }
    if (result == SSTEP_OK)
    {
        result = ratio(st->adc_vbus_mv_full, 1U, st->adc_full, vmv, &m->adc_v);
    }
    if (result == SSTEP_OK)
    {
        /* back-EMF per microstep/s: Kt 2 pi / (Ns M) / V_base */
        result = ratio(mo->ke_uv_s_per_rad, TWO_PI_NUM,
                       (uint64_t)TWO_PI_DEN * mo->full_steps_per_rev * c->microsteps, vmv * 1000U,
                       &m->ff_gain);
    }
    /* trip <= span <= 65535 because i_trip_ma <= i_full_scale_ma (check_config) */
    if ((result == SSTEP_OK) && (trip == 0U))
    {
        result = SSTEP_ERR_CONFIG;
    }
    if (result == SSTEP_OK)
    {
        result = set_currents(m, c->i_run_ma, c->i_hold_ma);
    }
    if (result == SSTEP_OK)
    {
        result = set_limits(m, c->v_max, c->accel, c->decel);
    }
    if (result == SSTEP_OK)
    {
        result = derive_stall(m);
    }
    if (result == SSTEP_OK)
    {
        m->i_trip_counts = (uint16_t)trip;
        m->vbus_max_raw = (uint16_t)((vmax_raw > 65535U) ? 65535U : vmax_raw);
        m->vbus_min_raw = (uint16_t)((vmin_raw > 65535U) ? 65535U : vmin_raw);
        m->ratio = (uint16_t)(c->fast_loop_hz / c->slow_loop_hz);
        m->hold_ticks = (uint32_t)(((uint64_t)c->hold_delay_ms * c->slow_loop_hz) / 1000U);
        m->angle_shift = (uint8_t)(14U - log2_u32(c->microsteps));
        derive_window(m);
        derive_slew(m);
        /* nominal until measured */
        (void)sstep_gain_from_ratio((int32_t)m->bus_num, (int32_t)m->bus_den, &m->inv_vbus);
    }
    return result;
}

/**
 * @brief  Last bus voltage sample outside the trip limits. Without a sample (0) the result is
 *         need_sample: sstep_enable() then leaves the check to the fast loop.
 */
static bool vbus_out_of_range(const struct sstep_ctx* m, bool need_sample)
{
    uint16_t raw = m->vbus_raw;
    bool out;

    if (raw == 0U)
    {
        out = need_sample;
    }
    else
    {
        out = ((m->vbus_min_raw != 0U) && (raw < m->vbus_min_raw)) ||
              ((m->vbus_max_raw != 0U) && (raw > m->vbus_max_raw));
    }
    return out;
}

#if (SSTEP_CFG_SELFTEST == 1)
/** @brief A self-test is running (the fast loop drives the outputs). */
static bool selftest_running(const struct sstep_ctx* m)
{
    return m->st.result == SSTEP_SELFTEST_RUNNING;
}

/** @brief Report the end of a self-test once (slow loop context). */
static void report_selftest(struct sstep_ctx* m)
{
    if (m->st.done != 0U)
    {
        m->st.done = 0U;
        if (m->cfg.hal.on_event != NULL)
        {
            m->cfg.hal.on_event(m->cfg.hal.context, SSTEP_EVENT_SELFTEST_DONE);
        }
    }
}

/** @brief One fast loop period of a running self-test (no trip in this period). */
static void selftest_period(struct sstep_ctx* m, int32_t da, int32_t db)
{
    uint16_t compare[SSTEP_LEGS_MAX];
    int32_t sign = 1;
    enum sstep_selftest_result r;

    if (m->cfg.stage.adc_i_span < 0)
    {
        sign = -1; /* inverting amplifier */
    }
    r = sstep_selftest_step(&m->st, sign * da, sign * db, compare);

    if (m->cfg.hal.pwm_write(m->cfg.hal.context, compare) < 0)
    {
        m->st.result = SSTEP_SELFTEST_ABORTED;
        m->st.done = 1U;
        shutdown(m, SSTEP_FAULT_HAL);
    }
    else if (r == SSTEP_SELFTEST_PASS)
    {
        m->st.done = 1U;
        if (m->cfg.hal.output_enable(m->cfg.hal.context, false) < 0)
        {
            shutdown(m, SSTEP_FAULT_HAL);
        }
    }
    else if (r != SSTEP_SELFTEST_RUNNING)
    {
        m->st.done = 1U;
        shutdown(m, SSTEP_FAULT_SELFTEST);
    }
    else
    {
        /* running */
    }
}
#endif /* SSTEP_CFG_SELFTEST */

/**
 * @brief  Sampling window: the legs may use the compare range 0 .. P - W only, which acts
 *         like a bus voltage of Vbus (P - W) / P. inv_vbus is scaled by bus_num / bus_den
 *         (P / (P - W)), the bus voltage of the applied phase voltages by bus_frac. With
 *         W = 0 both ratios are 1 / 1 and nothing changes.
 */
static void derive_window(struct sstep_ctx* m)
{
    uint16_t p = m->cfg.stage.pwm_period_ticks;
    uint16_t w = m->cfg.stage.sample_window_ticks;

    m->period_eff = (uint16_t)(p - w);
    m->bus_num = (w == 0U) ? 1U : p;
    m->bus_den = (w == 0U) ? 1U : m->period_eff;
    /* (P - W) / P with P >= 2, W <= P / 4: always representable */
    (void)sstep_gain_from_ratio((int32_t)m->bus_den, (int32_t)m->bus_num, &m->bus_frac);
}

/** @brief Current slew per slow period [pu], at least one mA; 0 = off. */
static void derive_slew(struct sstep_ctx* m)
{
    uint32_t slew = m->cfg.ctrl.i_slew_ma_per_ms;
    uint64_t per_period = ((uint64_t)slew * 1000U) / m->cfg.ctrl.slow_loop_hz;

    if (slew == 0U)
    {
        m->i_slew = SSTEP_REAL_ZERO;
    }
    else
    {
        if (per_period == 0U)
        {
            per_period = 1U;
        }
        if (per_period > m->cfg.stage.i_full_scale_ma)
        {
            per_period = m->cfg.stage.i_full_scale_ma;
        }
        (void)sstep_real_from_ratio((int32_t)per_period, (int32_t)m->cfg.stage.i_full_scale_ma,
                                    &m->i_slew);
    }
}

/** @brief Current amplitude of the next slow period: towards target by at most i_slew. */
static sstep_real_t slew_current(const struct sstep_ctx* m, sstep_real_t target)
{
    sstep_real_t now = m->i_amp;
    sstep_real_t next = target;

    if (m->cfg.ctrl.i_slew_ma_per_ms != 0U)
    {
        if (sstep_sub(target, now) > m->i_slew)
        {
            next = sstep_add(now, m->i_slew);
        }
        else if (sstep_sub(now, target) > m->i_slew)
        {
            next = sstep_sub(now, m->i_slew);
        }
        else
        {
            /* within one step: target */
        }
    }
    return next;
}

/** @brief Switch the outputs off and enter the fault state (any context). */
static void shutdown(struct sstep_ctx* m, uint16_t faults)
{
    m->faults = (uint16_t)(m->faults | faults);
    m->state = SSTEP_STATE_FAULT;
    (void)m->cfg.hal.output_enable(m->cfg.hal.context, false);
    if (m->cfg.hal.on_fault != NULL)
    {
        m->cfg.hal.on_fault(m->cfg.hal.context, faults);
    }
}

/** @brief Hand a setpoint to the fast loop (slow loop context). The fast loop never reads
 *         the buffer being written: sp_pending is cleared first (single core). */
static void publish(struct sstep_ctx* m, const struct sstep_setpoint* sp)
{
    uint8_t w;
    m->sp_pending = 0U;
    w = (uint8_t)(1U - m->sp_active);
    m->sp[w] = *sp;
    m->sp_pending = 1U;
}

/** @brief Outputs are on. */
static bool is_active(enum sstep_state s)
{
    return (s == SSTEP_STATE_HOLDING) || (s == SSTEP_STATE_MOVING);
}

/** @brief A command is executing. In hold mode the velocity is always zero: a move ends
 *         with v = 0 on the target and a stop becomes hold only at v = 0. */
static bool is_moving(const struct sstep_ctx* m)
{
    return (m->mo.mode != SSTEP_MODE_HOLD);
}

/** @brief Current control, power stage and PWM write of one fast loop period. */
static void control(struct sstep_ctx* m)
{
    struct sstep_sincos sc;
    sstep_real_t iref[2];
    sstep_acc_t ff[2];
    sstep_acc_t vu[2];
    sstep_real_t e[2];
    struct sstep_bridge_out out;
    sstep_acc_t vbus;

    if (m->sp_pending != 0U)
    {
        m->sp_active = (uint8_t)(1U - m->sp_active);
        m->cur = m->sp[m->sp_active];
        m->adv = 0U;
        m->sp_pending = 0U;
    }

    sc = sstep_sin_cos(m->cur.angle);
    iref[0] = sstep_mul(m->cur.i_amp, sc.cos);
    iref[1] = sstep_mul(m->cur.i_amp, sc.sin);
    ff[0] = sstep_acc_from_real(sstep_neg(sstep_mul(m->cur.ff, sc.sin)));
    ff[1] = sstep_acc_from_real(sstep_mul(m->cur.ff, sc.cos));

    if (m->cfg.ctrl.drive_mode == SSTEP_DRIVE_CURRENT)
    {
        for (uint32_t k = 0U; k < 2U; k++)
        {
            e[k] = sstep_sub(iref[k], m->i_meas[k]);
            vu[k] = sstep_acc_add(
                sstep_acc_add(sstep_gain_mul(&m->kp, e[k]), sstep_integ_to_acc(m->integ[k])),
                ff[k]);
        }
    }
    else
    {
        for (uint32_t k = 0U; k < 2U; k++)
        {
            vu[k] = sstep_acc_add(sstep_gain_mul(&m->r_pu, iref[k]), ff[k]);
        }
    }

    vbus = sstep_acc_clamp(sstep_gain_mul_int(&m->adc_v, (int32_t)m->vbus_raw), SSTEP_ACC_ZERO,
                           VBUS_PU_MAX);
    if (m->bus_num != m->bus_den)
    {
        /* sampling window: the stage acts like Vbus (P - W) / P on 0 .. P - W */
        vbus = sstep_gain_mul_acc(&m->bus_frac, vbus);
    }
    sstep_bridge_map(m->cfg.stage.topology, vu[0], vu[1], &m->cur.inv_vbus, vbus, m->period_eff,
                     &out);

    if (m->cfg.ctrl.drive_mode == SSTEP_DRIVE_CURRENT)
    {
        /* integ += Ki Ts e + Ts / Tt (v_applied - v_requested) */
        for (uint32_t k = 0U; k < 2U; k++)
        {
            m->integ[k] = sstep_integ_add(m->integ[k], &m->ki, e[k]);
            if (out.limited)
            {
                m->integ[k] =
                    sstep_integ_add_acc(m->integ[k], &m->kb, sstep_acc_add(out.v[k], -vu[k]));
            }
        }
        if (m->stall_ticks != 0U)
        {
            sstep_stall_observe(m, &sc, out.v);
        }
    }

    for (uint32_t k = 0U; k < SSTEP_LEGS_MAX; k++)
    {
        m->compare[k] = out.compare[k];
    }
    if (m->cfg.hal.pwm_write(m->cfg.hal.context, m->compare) < 0)
    {
        shutdown(m, SSTEP_FAULT_HAL);
    }
    else if (m->adv < m->ratio)
    {
        m->cur.angle += (sstep_angle_t)m->cur.dangle; /* modulo one turn */
        m->adv++;
    }
    else
    {
        /* the slow loop is late: hold the angle */
    }
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

int sstep_init(struct sstep* drv, const struct sstep_config* cfg)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result;

    if ((m == NULL) || (cfg == NULL))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        result = check_config(cfg);
        if (result == SSTEP_OK)
        {
            *m = (struct sstep_ctx){0}; /* no <string.h> needed */
            m->cfg = *cfg;
            result = derive(m);
            m->state = SSTEP_STATE_DISABLED;
            if (result == SSTEP_OK)
            {
                m->ready = SSTEP_READY;
            }
        }
    }
    return result;
}

int sstep_enable(struct sstep* drv)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (m->state == SSTEP_STATE_FAULT)
    {
        result = SSTEP_ERR_STATE;
    }
#if (SSTEP_CFG_SELFTEST == 1)
    else if (selftest_running(m))
    {
        result = SSTEP_ERR_STATE;
    }
#endif
    else if ((m->state == SSTEP_STATE_DISABLED) && vbus_out_of_range(m, false))
    {
        result = SSTEP_ERR_VBUS; /* not latched: the supply may come up later */
    }
    else if (m->state == SSTEP_STATE_DISABLED)
    {
        struct sstep_setpoint sp;
        m->mo.mode = SSTEP_MODE_HOLD;
        m->mo.vel = 0;
        m->mo.target = m->mo.pos;
        m->integ[0] = SSTEP_ACC_ZERO;
        m->integ[1] = SSTEP_ACC_ZERO;
        m->still = 0U;
        m->hold_active = false;
        sp.angle = raw_angle(m, m->mo.pos) + m->angle_offset;
        sp.dangle = 0;
        /* with a current slew the amplitude rises from 0 in the slow loop */
        m->i_amp = (m->cfg.ctrl.i_slew_ma_per_ms == 0U) ? m->i_run : SSTEP_REAL_ZERO;
        sp.i_amp = m->i_amp;
        sp.ff = SSTEP_REAL_ZERO;
        sp.inv_vbus = m->inv_vbus;
        m->cur = sp;
        m->adv = 0U;
        m->sp_pending = 0U;
        sstep_stall_reset(m);
        if (m->cfg.hal.output_enable(m->cfg.hal.context, true) < 0)
        {
            shutdown(m, SSTEP_FAULT_HAL);
            result = SSTEP_ERR_STATE;
        }
        else
        {
            m->state = SSTEP_STATE_HOLDING;
        }
    }
    else
    {
        /* already enabled */
    }
    return result;
}

int sstep_disable(struct sstep* drv)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
#if (SSTEP_CFG_SELFTEST == 1)
        if (selftest_running(m))
        {
            m->st.result = SSTEP_SELFTEST_NONE; /* stopped by the application */
        }
#endif
        if (m->state != SSTEP_STATE_FAULT)
        {
            m->state = SSTEP_STATE_DISABLED;
        }
        if (m->cfg.hal.output_enable(m->cfg.hal.context, false) < 0)
        {
            shutdown(m, SSTEP_FAULT_HAL);
        }
        m->mo.mode = SSTEP_MODE_HOLD;
        m->mo.vel = 0;
        m->mo.target = m->mo.pos;
    }
    return result;
}

int sstep_fast_loop(struct sstep* drv, const struct sstep_sample* sample)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

#if (SSTEP_CFG_ARG_CHECKS == 1)
    if ((m == NULL) || (sample == NULL))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
#endif
    {
        int32_t da = (int32_t)sample->i_a - (int32_t)m->cfg.stage.adc_i_zero;
        int32_t db = (int32_t)sample->i_b - (int32_t)m->cfg.stage.adc_i_zero;
        uint16_t faults = SSTEP_FAULT_NONE;
        m->i_meas[0] = sstep_acc_to_real(sstep_gain_mul_int(&m->adc_i, da));
        m->i_meas[1] = sstep_acc_to_real(sstep_gain_mul_int(&m->adc_i, db));
        m->vbus_raw = sample->vbus;

#if (SSTEP_CFG_SELFTEST == 1)
        if (is_active(m->state) || selftest_running(m))
#else
        if (is_active(m->state))
#endif
        {
            int32_t trip = (int32_t)m->i_trip_counts;
            int32_t dw = da + db; /* shared leg: iA + iB */
#if (SSTEP_CFG_SELFTEST == 1)
            bool checked = (m->cfg.ctrl.drive_mode == SSTEP_DRIVE_CURRENT) ||
                           m->cfg.ctrl.i_trip_voltage_drive || selftest_running(m);
#else
            bool checked =
                (m->cfg.ctrl.drive_mode == SSTEP_DRIVE_CURRENT) || m->cfg.ctrl.i_trip_voltage_drive;
#endif
            bool over = (da > trip) || (da < -trip) || (db > trip) || (db < -trip) ||
                        ((m->cfg.stage.topology == SSTEP_TOPOLOGY_SHARED_LEG) &&
                         ((dw > trip) || (dw < -trip)));
            if (checked && over)
            {
                faults |= SSTEP_FAULT_OVERCURRENT;
                if (m->stall_count != 0U)
                {
                    /* the slip can reach the trip before stall_time_ms: the position is lost */
                    faults |= SSTEP_FAULT_STALL;
                }
            }
            if ((m->vbus_max_raw != 0U) && (sample->vbus > m->vbus_max_raw))
            {
                faults |= SSTEP_FAULT_OVERVOLTAGE;
            }
            if ((m->vbus_min_raw != 0U) && (sample->vbus < m->vbus_min_raw))
            {
                faults |= SSTEP_FAULT_UNDERVOLTAGE;
            }

            if (faults != SSTEP_FAULT_NONE)
            {
#if (SSTEP_CFG_SELFTEST == 1)
                if (selftest_running(m))
                {
                    /* an overcurrent during the test is a short; a bus trip ends it */
                    m->st.result = ((faults & SSTEP_FAULT_OVERCURRENT) != 0U) ?
                                       SSTEP_SELFTEST_SHORT :
                                       SSTEP_SELFTEST_ABORTED;
                    m->st.done = 1U;
                    if ((faults & SSTEP_FAULT_OVERCURRENT) != 0U)
                    {
                        faults |= SSTEP_FAULT_SELFTEST;
                    }
                }
#endif
                shutdown(m, faults);
            }
#if (SSTEP_CFG_SELFTEST == 1)
            else if (selftest_running(m))
            {
                selftest_period(m, da, db);
            }
#endif
            else
            {
                control(m);
            }
        }
    }
    return result;
}

int sstep_slow_loop(struct sstep* drv)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (is_active(m->state))
    {
        struct sstep_setpoint sp;
        int64_t p0 = m->mo.pos;
        int64_t dp;
        int32_t vel;
        uint8_t flags = sstep_motion_step(&m->mo);
        uint16_t raw = m->vbus_raw;
        bool hold_event = false;

        if ((m->mo.mode == SSTEP_MODE_RUN) && (m->mo.v_run == 0) && (m->mo.vel == 0))
        {
            m->mo.mode = SSTEP_MODE_HOLD; /* stop completed */
            m->mo.target = m->mo.pos;
        }
        dp = m->mo.pos - p0;

        /* run current while moving, hold current after hold_delay periods without motion */
        if (dp != 0)
        {
            m->still = 0U;
            m->hold_active = false;
        }
        else
        {
            m->still++;
            if ((m->still >= m->hold_ticks) && (!m->hold_active))
            {
                m->hold_active = true;
                hold_event = true;
            }
        }

        if (raw != 0U)
        {
            struct sstep_gain inv;
            if (sstep_gain_from_ratio64(
                    (int64_t)m->cfg.stage.adc_full * (int64_t)m->cfg.stage.vbus_nominal_mv *
                        (int64_t)m->bus_num,
                    (int64_t)raw * (int64_t)m->cfg.stage.adc_vbus_mv_full * (int64_t)m->bus_den,
                    &inv) == SSTEP_OK)
            {
                m->inv_vbus = inv;
            }
        }

        sp.angle = raw_angle(m, p0) + m->angle_offset;
        sp.dangle = (int32_t)((dp * pow2_i64((uint32_t)m->angle_shift)) / (int64_t)m->ratio);
        m->i_amp = slew_current(m, m->hold_active ? m->i_hold : m->i_run);
        sp.i_amp = m->i_amp;
        vel = (int32_t)div_round(dp * (int64_t)m->cfg.ctrl.slow_loop_hz, SSTEP_POS_ONE);
        sp.ff = gain_apply_i32(&m->ff_gain, vel);
        sp.inv_vbus = m->inv_vbus;
        publish(m, &sp);

        /* a fault raised by the fast loop meanwhile sets faults before the state */
        if (m->faults == SSTEP_FAULT_NONE)
        {
            m->state = is_moving(m) ? SSTEP_STATE_MOVING : SSTEP_STATE_HOLDING;
        }
        else
        {
            m->state = SSTEP_STATE_FAULT;
        }

#if (SSTEP_CFG_SELFTEST == 1)
        report_selftest(m);
#endif
        if (m->cfg.hal.on_event != NULL)
        {
            if ((flags & SSTEP_MOTION_DONE) != 0U)
            {
                m->cfg.hal.on_event(m->cfg.hal.context, SSTEP_EVENT_MOVE_DONE);
            }
            if (hold_event)
            {
                m->cfg.hal.on_event(m->cfg.hal.context, SSTEP_EVENT_HOLD_CURRENT);
            }
        }

        /* stall decision on the period that just ended, then the command of the next one */
        if (m->stall_ticks != 0U)
        {
            if (sstep_stall_check(m))
            {
                shutdown(m, SSTEP_FAULT_STALL);
            }
            m->vel_cmd = vel;
            m->ff_cmd = sp.ff;
        }
    }
    else
    {
#if (SSTEP_CFG_SELFTEST == 1)
        report_selftest(m);
#endif
        m->mo.mode = SSTEP_MODE_HOLD;
        m->mo.vel = 0;
        m->mo.target = m->mo.pos;
    }
    return result;
}

int sstep_move_to(struct sstep* drv, int32_t position)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (!is_active(m->state))
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        m->mo.target = (int64_t)position * SSTEP_POS_ONE;
        m->mo.mode = SSTEP_MODE_MOVE;
    }
    return result;
}

int sstep_move_by(struct sstep* drv, int32_t delta)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        int64_t t = div_round(m->mo.target, SSTEP_POS_ONE) + (int64_t)delta;
        if ((t > (int64_t)INT32_MAX) || (t < (int64_t)INT32_MIN))
        {
            result = SSTEP_ERR_PARAM;
        }
        else
        {
            result = sstep_move_to(drv, (int32_t)t);
        }
    }
    return result;
}

int sstep_run(struct sstep* drv, int32_t velocity)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if ((velocity > (int32_t)m->cfg.ctrl.v_max) || (velocity < -(int32_t)m->cfg.ctrl.v_max))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (!is_active(m->state))
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        m->mo.v_run = (int32_t)div_round((int64_t)velocity * SSTEP_POS_ONE,
                                         (int64_t)m->cfg.ctrl.slow_loop_hz);
        m->mo.mode = SSTEP_MODE_RUN;
    }
    return result;
}

int sstep_stop(struct sstep* drv)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (is_active(m->state) && is_moving(m))
    {
        m->mo.v_run = 0;
        m->mo.mode = SSTEP_MODE_RUN;
    }
    else
    {
        /* nothing to stop */
    }
    return result;
}

int sstep_set_position(struct sstep* drv, int32_t position)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (is_moving(m))
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        int64_t p = (int64_t)position * SSTEP_POS_ONE;
        /* keep the electrical angle: the rotor must not move */
        m->angle_offset += raw_angle(m, m->mo.pos) - raw_angle(m, p);
        m->mo.pos = p;
        m->mo.target = p;
    }
    return result;
}

int sstep_set_current(struct sstep* drv, uint32_t run_ma, uint32_t hold_ma)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        result = set_currents(m, run_ma, hold_ma);
    }
    return result;
}

int sstep_set_motion_limits(struct sstep* drv, uint32_t v_max, uint32_t accel, uint32_t decel)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        result = set_limits(m, v_max, accel, decel);
    }
    return result;
}

int sstep_report_fault(struct sstep* drv, uint16_t faults)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else if (faults != SSTEP_FAULT_NONE)
    {
        shutdown(m, faults);
    }
    else
    {
        /* nothing reported */
    }
    return result;
}

int sstep_clear_faults(struct sstep* drv)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        m->faults = SSTEP_FAULT_NONE;
        m->state = SSTEP_STATE_DISABLED;
    }
    return result;
}

int sstep_get_status(const struct sstep* drv, struct sstep_status* status)
{
    const struct sstep_ctx* m = sstep_ctx_of_const(drv);
    int result = SSTEP_OK;

    if ((m == NULL) || (status == NULL))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
    else
    {
        int32_t base = (int32_t)m->cfg.stage.i_full_scale_ma;
        status->state = m->state;
        status->faults = m->faults;
        status->position = (int32_t)div_round(m->mo.pos, SSTEP_POS_ONE);
        status->target = (int32_t)div_round(m->mo.target, SSTEP_POS_ONE);
        status->velocity = (int32_t)div_round(
            (int64_t)m->mo.vel * (int64_t)m->cfg.ctrl.slow_loop_hz, SSTEP_POS_ONE);
        status->i_a_ma = sstep_real_to_units(sstep_acc_from_real(m->i_meas[0]), base);
        status->i_b_ma = sstep_real_to_units(sstep_acc_from_real(m->i_meas[1]), base);
        status->vbus_mv = (uint32_t)(((uint64_t)m->vbus_raw * m->cfg.stage.adc_vbus_mv_full) /
                                     m->cfg.stage.adc_full);
        sstep_stall_status(m, status);
#if (SSTEP_CFG_SELFTEST == 1)
        status->selftest = m->st.result;
        status->selftest_phase = m->st.coil;
        status->selftest_i_ma =
            (int32_t)(((int64_t)m->st.i_own * base) / ((m->cfg.stage.adc_i_span < 0) ?
                                                           -(int64_t)m->cfg.stage.adc_i_span :
                                                           (int64_t)m->cfg.stage.adc_i_span));
        status->selftest_duty = (uint16_t)(m->st.duty_q8 / 256U);
#else
        status->selftest = SSTEP_SELFTEST_NONE;
        status->selftest_phase = 0U;
        status->selftest_i_ma = 0;
        status->selftest_duty = 0U;
#endif
    }
    return result;
}

int sstep_selftest(struct sstep* drv, uint32_t i_test_ma)
{
    struct sstep_ctx* m = sstep_ctx_of(drv);
    int result = SSTEP_OK;

    if (m == NULL)
    {
        result = SSTEP_ERR_PARAM;
    }
    else if (m->ready != SSTEP_READY)
    {
        result = SSTEP_ERR_STATE;
    }
#if (SSTEP_CFG_SELFTEST == 0)
    else
    {
        (void)i_test_ma;
        result = SSTEP_ERR_UNSUPPORTED;
    }
#else
    else if ((i_test_ma == 0U) || (i_test_ma > m->cfg.ctrl.i_run_ma))
    {
        result = SSTEP_ERR_PARAM;
    }
    else if ((m->state != SSTEP_STATE_DISABLED) || selftest_running(m))
    {
        result = SSTEP_ERR_STATE;
    }
    else if (vbus_out_of_range(m, true))
    {
        result = SSTEP_ERR_VBUS;
    }
    else
    {
        sstep_selftest_setup(&m->st, &m->cfg, m->vbus_raw, i_test_ma);
        if (m->cfg.hal.output_enable(m->cfg.hal.context, true) < 0)
        {
            m->st.result = SSTEP_SELFTEST_ABORTED;
            shutdown(m, SSTEP_FAULT_HAL);
            result = SSTEP_ERR_STATE;
        }
    }
#endif /* SSTEP_CFG_SELFTEST */
    return result;
}
