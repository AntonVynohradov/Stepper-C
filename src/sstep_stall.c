/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Sensorless stall detection: back-EMF observer (fast loop), decision (slow loop).
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

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** 2 pi = 710 / 113, as in the gain derivation. */
#define TWO_PI_NUM 710U
#define TWO_PI_DEN 113U

/** CORDIC iterations of atan2_turn() (last step atan(2^-15) = 0.0017 deg). */
#define CORDIC_STEPS 16U

/** The CORDIC operands are scaled up to at least 2^27 for resolution; the CORDIC gain
    (1.65 * sqrt(2)) keeps them below 2^30. */
#define CORDIC_NORM ((int32_t)0x08000000)

/** Half a turn of ::sstep_angle_t. */
#define HALF_TURN 0x80000000U

/** One turn of ::sstep_angle_t as a 64-bit value. */
#define TURN ((int64_t)4294967296)

/** Fixed point one of sstep_ctx::emf_off (mV * 2^16). */
#define OFF_ONE ((int64_t)65536)

/** Centidegrees per turn. */
#define CDEG_PER_TURN 36000

/* ==============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static int32_t emf_mv(const struct sstep_ctx* m, uint32_t axis);
static int64_t abs_i64(int64_t v);
static uint64_t muldiv_u64(uint64_t a, uint64_t b, uint64_t d);
static sstep_angle_t atan2_turn(int32_t y, int32_t x);

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

/** @brief Filtered back-EMF of one axis (0 = d, 1 = q) [mV]. */
static int32_t emf_mv(const struct sstep_ctx* m, uint32_t axis)
{
    return sstep_real_to_units(sstep_integ_to_acc(m->emf[axis]),
                               (int32_t)m->cfg.stage.vbus_nominal_mv);
}

/** @brief |v| for |v| < 2^63. */
static int64_t abs_i64(int64_t v)
{
    return (v < 0) ? -v : v;
}

/** @brief a * b / d (truncated) without overflow for a < 2^42, b < 2^25, 0 < d < 2^35. */
static uint64_t muldiv_u64(uint64_t a, uint64_t b, uint64_t d)
{
    uint64_t q = a / d;
    uint64_t r = a % d;
    return (q * b) + ((r * b) / d);
}

/**
 * @brief  Angle of the vector (x, y), atan2(y, x), in 2^-32 turn (CORDIC, vectoring mode).
 *         0 for the zero vector.
 * @param  y, x  Components, |x|, |y| < 2^27.
 */
static sstep_angle_t atan2_turn(int32_t y, int32_t x)
{
    /* atan(2^-i) in 2^-32 turn, i = 0 .. CORDIC_STEPS - 1. Read-only (flash). */
    static const uint32_t atan_table[CORDIC_STEPS] = {
        536870912U, 316933406U, 167458907U, 85004756U, 42667331U, 21354465U, 10679838U, 5340245U,
        2670163U,   1335087U,   667544U,    333772U,   166886U,   83443U,    41722U,    20861U};
    int32_t xs = x;
    int32_t ys = y;
    int32_t pw = 1; /* 2^i */
    sstep_angle_t a = 0U;
    uint32_t steps = CORDIC_STEPS;

    if ((x == 0) && (y == 0))
    {
        steps = 0U; /* the zero vector: angle 0 */
    }
    if (xs < 0)
    {
        /* rotate by half a turn into the right half plane */
        xs = -xs;
        ys = -ys;
        a = HALF_TURN;
    }
    while ((steps != 0U) && (xs < CORDIC_NORM) && (ys < CORDIC_NORM) && (ys > -CORDIC_NORM))
    {
        xs *= 2; /* at most 27 times */
        ys *= 2;
    }
    for (uint32_t i = 0U; i < steps; i++)
    {
        int32_t dx = ys / pw; /* truncates toward zero for either sign */
        int32_t dy = xs / pw;
        if (ys >= 0)
        {
            xs += dx;
            ys -= dy;
            a += atan_table[i];
        }
        else
        {
            xs -= dx;
            ys += dy;
            a -= atan_table[i];
        }
        pw *= 2;
    }
    return a;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS (library internal)
 * =============================================================================*/

void sstep_stall_reset(struct sstep_ctx* m)
{
    for (uint32_t k = 0U; k < 2U; k++)
    {
        m->i_prev[k] = m->i_meas[k];
        m->v_hist[0][k] = SSTEP_ACC_ZERO;
        m->v_hist[1][k] = SSTEP_ACC_ZERO;
        m->emf[k] = SSTEP_ACC_ZERO;
        m->emf_off[k] = 0;
    }
    m->vel_cmd = 0;
    m->ff_cmd = SSTEP_REAL_ZERO;
    m->stall_count = 0U;
    m->stall_hold = 0U;
}

void sstep_stall_observe(struct sstep_ctx* m, const struct sstep_sincos* sc,
                         const sstep_acc_t v_applied[2])
{
    sstep_real_t e[2];
    sstep_acc_t ed;
    sstep_acc_t eq;

    /* e = v(k-2) - R (i(k) + i(k-1)) / 2 - L (i(k) - i(k-1)) / Ts
         = v(k-2) - (L / Ts + R / 2) i(k) + (L / Ts - R / 2) i(k-1), per phase, in +/-1 pu */
    for (uint32_t k = 0U; k < 2U; k++)
    {
        sstep_acc_t x = sstep_acc_add(m->v_hist[1][k], -sstep_gain_mul(&m->emf_a, m->i_meas[k]));
        e[k] = sstep_acc_to_real(sstep_acc_add(x, sstep_gain_mul(&m->emf_b, m->i_prev[k])));
        m->i_prev[k] = m->i_meas[k];
        m->v_hist[1][k] = m->v_hist[0][k];
        m->v_hist[0][k] = v_applied[k];
    }

    /* into the commanded frame: d along the current reference, q 90 deg ahead */
    ed = sstep_acc_from_real(sstep_add(sstep_mul(e[0], sc->cos), sstep_mul(e[1], sc->sin)));
    eq = sstep_acc_from_real(sstep_sub(sstep_mul(e[1], sc->cos), sstep_mul(e[0], sc->sin)));

    /* first-order low-pass: y += Ts / tau (x - y) */
    m->emf[0] = sstep_integ_add_acc(m->emf[0], &m->emf_alpha,
                                    sstep_acc_add(ed, -sstep_integ_to_acc(m->emf[0])));
    m->emf[1] = sstep_integ_add_acc(m->emf[1], &m->emf_alpha,
                                    sstep_acc_add(eq, -sstep_integ_to_acc(m->emf[1])));
}

bool sstep_stall_check(struct sstep_ctx* m)
{
    int64_t ed = (int64_t)emf_mv(m, 0U);
    int64_t eq = (int64_t)emf_mv(m, 1U);
    /* expected back-EMF of the command, on the q axis: Ke w (signed) */
    int64_t eref = (int64_t)sstep_real_to_units(sstep_acc_from_real(m->ff_cmd),
                                                (int32_t)m->cfg.stage.vbus_nominal_mv);
    int64_t vel = (int64_t)m->vel_cmd;
    bool bad;

    if (abs_i64(vel) >= (int64_t)m->cfg.ctrl.stall_speed)
    {
        /* in motion: back-EMF along the commanded motion below sync_pct of Ke w, with the
           d axis inside the window of a rotor that does not turn: resistance error only
           (mostly positive), so that a rotor under load is not taken for a blocked one */
        int64_t along = (vel > 0) ? eq : -eq;
        int64_t er = abs_i64(eref);
        int64_t ri = ((int64_t)m->cfg.motor.r_mohm * (int64_t)m->cfg.ctrl.i_run_ma) / 1000;
        bad = ((along * 100) < ((int64_t)m->cfg.ctrl.stall_sync_pct * er)) &&
              (ed <= ((er / 3) + ((ri * 2) / 5))) && (ed >= -((er / 3) + (ri / 10)));
        /* a reversed back-EMF is beyond the pull-out: a slipping rotor passes here once per
           turn of the vector, and the condition stays true in between */
        if ((along * 2) < -(er + (2 * (int64_t)m->stall_emf_mv)))
        {
            m->stall_hold = m->stall_ticks / 2U;
        }
    }
    else
    {
        /* at rest or slow: the rotor turns faster than the command by more than
           stall_speed. The deviation from the expected back-EMF averages zero while the
           rotor follows: its running mean (time constant stall_time) is the observer error
           (resistance, dead time) and is subtracted; a slip makes the vector rotate. */
        int64_t lim = (int64_t)m->stall_emf_mv;
        int64_t dev[2];
        dev[0] = ed;
        dev[1] = eq - eref;
        for (uint32_t k = 0U; k < 2U; k++)
        {
            m->emf_off[k] += ((dev[k] * OFF_ONE) - m->emf_off[k]) / (int64_t)m->stall_ticks;
            dev[k] -= m->emf_off[k] / OFF_ONE;
        }
        bad = (((dev[0] * dev[0]) + (dev[1] * dev[1])) > (lim * lim));
    }

    if (m->stall_hold != 0U)
    {
        m->stall_hold--;
        bad = true;
    }

    if (bad)
    {
        m->stall_count++;
    }
    else if (m->stall_count > 0U)
    {
        m->stall_count--;
    }
    else
    {
        /* idle */
    }
    return (m->stall_count >= m->stall_ticks);
}

void sstep_stall_status(const struct sstep_ctx* m, struct sstep_status* status)
{
    if (m->stall_ticks == 0U)
    {
        status->emf_d_mv = 0;
        status->emf_q_mv = 0;
        status->rotor_velocity = 0;
        status->load_angle_cdeg = 0;
        status->stall_level_pct = 0U;
    }
    else
    {
        int64_t ed = (int64_t)emf_mv(m, 0U);
        int64_t eq = (int64_t)emf_mv(m, 1U);
        /* direction of the command, or of the back-EMF at rest */
        int64_t s = 1;
        if ((m->vel_cmd < 0) || ((m->vel_cmd == 0) && (eq < 0)))
        {
            s = -1;
        }
        int64_t sq = (ed * ed) + (eq * eq);
        uint64_t mag = sstep_isqrt64((uint64_t)sq);
        /* speed = |e| / Ke [microsteps/s] = |e| Ns M / (2 pi Ke) */
        uint64_t ns_m =
            (uint64_t)m->cfg.motor.full_steps_per_rev * (uint64_t)m->cfg.ctrl.microsteps;
        uint64_t speed = muldiv_u64(mag * 1000U * TWO_PI_DEN, ns_m,
                                    (uint64_t)TWO_PI_NUM * m->cfg.motor.ke_uv_s_per_rad);
        sstep_angle_t a = atan2_turn((int32_t)(s * ed), (int32_t)(s * eq)); /* |e| < 2^24 */
        int64_t a_signed = (int64_t)a - ((a >= HALF_TURN) ? TURN : 0);
        int64_t v = (speed > (uint64_t)INT32_MAX) ? (int64_t)INT32_MAX : (int64_t)speed;

        status->emf_d_mv = (int32_t)ed;
        status->emf_q_mv = (int32_t)eq;
        status->rotor_velocity = (int32_t)((eq >= 0) ? v : -v);
        status->load_angle_cdeg = (int32_t)((a_signed * CDEG_PER_TURN) / TURN);
        /* the counter stops at the trip (the outputs are off): at most 100 % */
        status->stall_level_pct = (uint8_t)(((uint64_t)m->stall_count * 100U) / m->stall_ticks);
    }
}
