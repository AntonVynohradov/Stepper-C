/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Numeric backend: one set of types and operations for fixed point (Q15) and float.
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


#ifndef SSTEP_MATH_H
#define SSTEP_MATH_H

/**
 * @defgroup sstep_math Numeric backend
 * @brief    Types and operations that hide whether the library computes in fixed
 *           point or in float.
 *
 * The control code is written once against these types and operations; the backend
 * is selected at build time with ::SSTEP_CFG_MATH_BACKEND.
 *
 * **Per-unit signals.** Inside the control loops every electrical signal is normalised
 * to a base value computed by sstep_init():
 * - current: 1.0 = power stage current full scale (`i_full_scale_ma`),
 * - voltage: 1.0 = nominal bus voltage (`vbus_nominal_mv`),
 * - duty cycle: 1.0 = 100 %.
 *
 * Both backends therefore see values in about -1..+1, which makes Q15 exact enough and
 * lets the two backends be compared sample by sample in the tests.
 *
 * | Type | Fixed point backend | Float backend |
 * | --- | --- | --- |
 * | ::sstep_real_t | `int16_t`, Q15 (1.0 = 32768, range -1 .. +0.99997) | `float`, same range |
 * | ::sstep_acc_t  | `int32_t`, Q15 with 16 integer bits (range +/-65536) | `float`, same range |
 * | ::sstep_angle_t | `uint32_t`, one electrical turn = 2^32 | same |
 * | `struct sstep_gain` | Q15 mantissa and binary exponent | `float` |
 *
 * **Same behaviour in both backends.** Every operation saturates to the ranges in the
 * table instead of overflowing, in both backends, so that a test can compare them.
 * Angles wrap by design.
 *
 * **Speed.** The per-sample operations are `static inline` and branch-light; the
 * fixed point backend uses only 32-bit multiplications (no 64-bit arithmetic), so it
 * suits Cortex-M0. Functions that convert engineering units (`*_from_ratio`,
 * sstep_real_to_units()) use 64-bit integers and division: call them at
 * initialisation or from the slow loop, not from the fast loop.
 * @{
 */

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdint.h>

#include "sstep_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

/** @brief Electrical angle: one full electrical turn (4 full steps) = 2^32. */
typedef uint32_t sstep_angle_t;

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

/** @brief Per-unit signal (Q15). */
typedef int16_t sstep_real_t;

/** @brief Per-unit accumulator (Q15 in 32 bits), used by integrators and filters. */
typedef int32_t sstep_acc_t;

/**
 * @brief Gain of any magnitude: value = mant / 2^15 * 2^exp.
 *        Created by sstep_gain_from_ratio(); the mantissa is normalised
 *        (16384 <= |mant| <= 32767) for the best resolution, or 0.
 */
struct sstep_gain
{
    int16_t mant; /**< Q15 mantissa. */
    int8_t exp;   /**< Binary exponent, ::SSTEP_GAIN_EXP_MIN .. ::SSTEP_GAIN_EXP_MAX. */
};

/** @brief Number of fractional bits of ::sstep_real_t and ::sstep_acc_t. */
#define SSTEP_REAL_FRAC_BITS 15

/** @brief Largest ::sstep_real_t (+0.99997). */
#define SSTEP_REAL_MAX ((sstep_real_t)INT16_MAX)

/** @brief Smallest ::sstep_real_t (-1.0). */
#define SSTEP_REAL_MIN ((sstep_real_t)INT16_MIN)

/** @brief Largest ::sstep_acc_t (+65536 - 2^-15). */
#define SSTEP_ACC_MAX ((sstep_acc_t)INT32_MAX)

/** @brief Smallest ::sstep_acc_t (-65536). */
#define SSTEP_ACC_MIN ((sstep_acc_t)INT32_MIN)

/** @brief Accumulator constant from a floating literal (initializers only). */
#define SSTEP_ACC_C(x) ((sstep_acc_t)((x) * 32768.0))

/** @brief Zero as a signal. */
#define SSTEP_REAL_ZERO ((sstep_real_t)0)

/** @brief Zero as an accumulator. */
#define SSTEP_ACC_ZERO ((sstep_acc_t)0)

/** @brief 0.5 as an accumulator. */
#define SSTEP_ACC_HALF ((sstep_acc_t)16384)

/** @brief 1.0 as an accumulator. */
#define SSTEP_ACC_ONE ((sstep_acc_t)32768)

/** @brief Largest magnitude accepted by sstep_gain_mul_acc() and the integrator
 *         updates from accumulators (4 pu - 2^-15); larger inputs are saturated. */
#define SSTEP_ACC_IN_MAX ((sstep_acc_t)131071)

/** @brief Largest magnitude accepted by sstep_acc_scale() (2 pu - 2^-15). */
#define SSTEP_ACC_SCALE_MAX ((sstep_acc_t)65535)

/**
 * @brief Integrator: Q27 in 32 bits (range +/-16 pu, resolution 7.5e-9 pu). The extra
 *        12 fractional bits keep small increments (Ki * Ts * e is often below one Q15
 *        LSB) instead of rounding them away.
 */
typedef int32_t sstep_integ_t;

/** @brief Fractional bits of ::sstep_integ_t. */
#define SSTEP_INTEG_FRAC_BITS 27

/** @brief Constant from a floating literal, saturated to -1.0 .. +0.99997.
 *         An arithmetic constant expression: use it in initializers of static or
 *         `const` objects, where the compiler evaluates it and no floating point code
 *         is generated. It is not an integer constant expression (C11 6.6), so it cannot
 *         be used in `#if`, `case` or `static_assert`. */
#define SSTEP_REAL_C(x)              \
    (((x) >= 1.0) ? SSTEP_REAL_MAX : \
                    (((x) <= -1.0) ? SSTEP_REAL_MIN : (sstep_real_t)((x) * 32768.0)))

#else /* SSTEP_MATH_BACKEND_FLOAT */

/** @brief Per-unit signal. */
typedef float sstep_real_t;

/** @brief Per-unit accumulator. */
typedef float sstep_acc_t;

/** @brief Gain of any magnitude. */
struct sstep_gain
{
    float value; /**< Gain. */
};

/** @brief Largest ::sstep_real_t (same as Q15). */
#define SSTEP_REAL_MAX (0.999969482421875f)

/** @brief Smallest ::sstep_real_t. */
#define SSTEP_REAL_MIN (-1.0f)

/** @brief Largest ::sstep_acc_t: the largest float below 65536 (the fixed point limit
 *         65536 - 2^-15 is not representable in single precision). */
#define SSTEP_ACC_MAX (65535.99609375f)

/** @brief Smallest ::sstep_acc_t. */
#define SSTEP_ACC_MIN (-65536.0f)

/** @brief Accumulator constant. */
#define SSTEP_ACC_C(x) ((sstep_acc_t)(x))

/** @brief Zero as a signal. */
#define SSTEP_REAL_ZERO (0.0f)

/** @brief Zero as an accumulator. */
#define SSTEP_ACC_ZERO (0.0f)

/** @brief 0.5 as an accumulator. */
#define SSTEP_ACC_HALF (0.5f)

/** @brief 1.0 as an accumulator. */
#define SSTEP_ACC_ONE (1.0f)

/** @brief Largest magnitude accepted by sstep_gain_mul_acc() and the integrator
 *         updates from accumulators (4 pu - 2^-15); larger inputs are saturated. */
#define SSTEP_ACC_IN_MAX (3.999969482421875f)

/** @brief Largest magnitude accepted by sstep_acc_scale() (2 pu - 2^-15). */
#define SSTEP_ACC_SCALE_MAX (1.999969482421875f)

/** @brief Integrator (same range as the fixed point integrator, +/-16 pu). */
typedef float sstep_integ_t;

/** @brief Constant from a floating literal, saturated to the same range as the fixed
 *         point backend so that both backends behave identically. */
#define SSTEP_REAL_C(x) \
    (((x) >= 1.0) ? SSTEP_REAL_MAX : (((x) <= -1.0) ? SSTEP_REAL_MIN : (sstep_real_t)(x)))

#endif /* SSTEP_CFG_MATH_BACKEND */

/** @brief Smallest gain exponent. */
#define SSTEP_GAIN_EXP_MIN (-16)

/** @brief Largest gain exponent (gains up to 32767). */
#define SSTEP_GAIN_EXP_MAX 15

/** @brief Sine and cosine of one angle, see sstep_sin_cos(). */
struct sstep_sincos
{
    sstep_real_t sin; /**< Sine. */
    sstep_real_t cos; /**< Cosine. */
};

/* ==============================================================================
 * FIXED POINT OPERATIONS
 * =============================================================================*/

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

/**
 * @brief  Arithmetic shift right (rounds toward minus infinity), written with unsigned
 *         shifts only (MISRA C:2012 Rule 10.1); compilers emit one ASR instruction.
 * @param  x  Value.
 * @param  n  Shift, 0..31.
 * @return floor(x / 2^n).
 */
static inline int32_t sstep_asr(int32_t x, uint32_t n)
{
    int32_t result;
    uint32_t u;
    if (x >= 0)
    {
        u = (uint32_t)x >> n;
        result = (int32_t)u;
    }
    else
    {
        u = (uint32_t)(-(x + 1)) >> n;
        result = -(int32_t)u - 1;
    }
    return result;
}

/**
 * @brief  Saturate a 32-bit value to ::sstep_real_t.
 * @param  x  Value in Q15.
 * @return x limited to ::SSTEP_REAL_MIN .. ::SSTEP_REAL_MAX.
 */
static inline sstep_real_t sstep_sat(int32_t x)
{
    int32_t r = x;
    if (r > (int32_t)INT16_MAX)
    {
        r = (int32_t)INT16_MAX;
    }
    else if (r < (int32_t)INT16_MIN)
    {
        r = (int32_t)INT16_MIN;
    }
    else
    {
        /* in range */
    }
    return (sstep_real_t)r;
}

/** @brief Saturating a + b. */
static inline sstep_real_t sstep_add(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat((int32_t)a + (int32_t)b);
}

/** @brief Saturating a - b. */
static inline sstep_real_t sstep_sub(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat((int32_t)a - (int32_t)b);
}

/** @brief Saturating -a (-(-1.0) gives +0.99997). */
static inline sstep_real_t sstep_neg(sstep_real_t a)
{
    return sstep_sat(-(int32_t)a);
}

/** @brief Saturating |a|. */
static inline sstep_real_t sstep_abs(sstep_real_t a)
{
    return sstep_sat((a < 0) ? -(int32_t)a : (int32_t)a);
}

/** @brief a * b, rounded to nearest; (-1.0) * (-1.0) saturates to +0.99997. */
static inline sstep_real_t sstep_mul(sstep_real_t a, sstep_real_t b)
{
    int32_t p = (int32_t)a * (int32_t)b; /* |p| <= 2^30: no overflow */
    return sstep_sat(sstep_asr(p + 0x4000, 15U));
}

/** @brief Convert a signal to an accumulator (exact). */
static inline sstep_acc_t sstep_acc_from_real(sstep_real_t a)
{
    return (sstep_acc_t)a;
}

/** @brief Convert an accumulator to a signal, saturating. */
static inline sstep_real_t sstep_acc_to_real(sstep_acc_t a)
{
    return sstep_sat(a);
}

/** @brief Saturating accumulator a + b. */
static inline sstep_acc_t sstep_acc_add(sstep_acc_t a, sstep_acc_t b)
{
    sstep_acc_t r;
    if ((b > 0) && (a > (INT32_MAX - b)))
    {
        r = INT32_MAX;
    }
    else if ((b < 0) && (a < (INT32_MIN - b)))
    {
        r = (int32_t)INT32_MIN;
    }
    else
    {
        r = a + b;
    }
    return r;
}

/** @brief Accumulator limited to lo .. hi (requires lo <= hi). */
static inline sstep_acc_t sstep_acc_clamp(sstep_acc_t x, sstep_acc_t lo, sstep_acc_t hi)
{
    sstep_acc_t r = x;
    if (r > hi)
    {
        r = hi;
    }
    else if (r < lo)
    {
        r = lo;
    }
    else
    {
        /* in range */
    }
    return r;
}

/**
 * @brief  Gain times signal: g * x, rounded to nearest.
 * @param  g  Gain from sstep_gain_from_ratio().
 * @param  x  Signal.
 * @return Product as an accumulator (cannot overflow: |g * x| < 32768).
 */
static inline sstep_acc_t sstep_gain_mul(const struct sstep_gain* g, sstep_real_t x)
{
    int32_t p = (int32_t)x * (int32_t)g->mant; /* Q30, |p| <= 2^30 */
    int32_t s = 15 - (int32_t)g->exp;          /* 0 .. 31 */
    uint32_t sh = (uint32_t)s;
    int32_t r;
    if (sh == 0U)
    {
        r = p;
    }
    else
    {
        r = sstep_asr(sstep_asr(p, sh - 1U) + 1, 1U); /* round half up, no overflow */
    }
    return r;
}

/**
 * @brief  Gain times integer: g * n as a per-unit accumulator, saturating. Used for
 *         ADC counts: with g = 1 / (counts per unit), the result is the signal in pu.
 * @param  g  Gain from sstep_gain_from_ratio().
 * @param  n  Integer, -32768 .. 32767 (larger values are saturated first).
 * @return g * n in Q15, saturated to the accumulator range.
 */
static inline sstep_acc_t sstep_gain_mul_int(const struct sstep_gain* g, int32_t n)
{
    int32_t p = (int32_t)sstep_sat(n) * (int32_t)g->mant; /* |p| <= 2^30 */
    int32_t e = (int32_t)g->exp;
    int32_t r;
    if (e >= 0)
    {
        int32_t lim = sstep_asr(INT32_MAX, (uint32_t)e);
        if (p > lim)
        {
            r = INT32_MAX;
        }
        else if (p < -lim)
        {
            r = (int32_t)INT32_MIN;
        }
        else
        {
            uint32_t pw = (uint32_t)1U << (uint32_t)e;
            r = p * (int32_t)pw; /* |p| <= lim: no overflow */
        }
    }
    else
    {
        uint32_t sh = (uint32_t)(-e); /* 1 .. 16 */
        r = sstep_asr(sstep_asr(p, sh - 1U) + 1, 1U);
    }
    return r;
}


/**
 * @brief  Shift with rounding: sh > 0 divides by 2^sh (round half up), sh < 0
 *         multiplies by 2^-sh with saturation.
 * @param  p   Value.
 * @param  sh  Shift, -31 .. 31.
 * @return Shifted value.
 */
static inline int32_t sstep_shift_round(int32_t p, int32_t sh)
{
    int32_t r;
    if (sh > 0)
    {
        uint32_t n = (uint32_t)sh;
        r = sstep_asr(sstep_asr(p, n - 1U) + 1, 1U);
    }
    else if (sh == 0)
    {
        r = p;
    }
    else
    {
        uint32_t n = (uint32_t)(-sh);
        int32_t lim = sstep_asr(INT32_MAX, n);
        if (p > lim)
        {
            r = INT32_MAX;
        }
        else if (p < -lim)
        {
            r = (int32_t)INT32_MIN;
        }
        else
        {
            uint32_t pw = (uint32_t)1U << n;
            r = p * (int32_t)pw; /* |p| <= lim: no overflow */
        }
    }
    return r;
}

/** @brief Saturating |a| of an accumulator. */
static inline sstep_acc_t sstep_acc_abs(sstep_acc_t a)
{
    return (a < 0) ? ((a == INT32_MIN) ? INT32_MAX : -a) : a;
}

/** @brief a / 2 (rounds toward minus infinity). */
static inline sstep_acc_t sstep_acc_halve(sstep_acc_t a)
{
    return sstep_asr(a, 1U);
}

/** @brief Limit an accumulator to +/-::SSTEP_ACC_IN_MAX. */
static inline sstep_acc_t sstep_acc_in(sstep_acc_t x)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_IN_MAX, SSTEP_ACC_IN_MAX);
}

/**
 * @brief  Gain times accumulator: g * x for |x| < 4 pu (saturated first). The two
 *         least significant bits of x are dropped (resolution 1.2e-4 pu).
 */
static inline sstep_acc_t sstep_gain_mul_acc(const struct sstep_gain* g, sstep_acc_t x)
{
    int32_t p = sstep_asr(sstep_acc_in(x), 2U) * (int32_t)g->mant; /* |p| < 2^30 */
    return sstep_shift_round(p, 13 - (int32_t)g->exp);
}

/** @brief Integrator + g * x (x a signal), saturating. */
static inline sstep_integ_t sstep_integ_add(sstep_integ_t integ, const struct sstep_gain* g,
                                            sstep_real_t x)
{
    int32_t p = (int32_t)x * (int32_t)g->mant; /* Q30, |p| <= 2^30 */
    return sstep_acc_add(integ, sstep_shift_round(p, 3 - (int32_t)g->exp));
}

/** @brief Integrator + g * x (x an accumulator, |x| < 4 pu), saturating. */
static inline sstep_integ_t sstep_integ_add_acc(sstep_integ_t integ, const struct sstep_gain* g,
                                                sstep_acc_t x)
{
    int32_t p = sstep_asr(sstep_acc_in(x), 2U) * (int32_t)g->mant;
    return sstep_acc_add(integ, sstep_shift_round(p, 1 - (int32_t)g->exp));
}

/** @brief Integrator value as an accumulator (rounded). */
static inline sstep_acc_t sstep_integ_to_acc(sstep_integ_t integ)
{
    return sstep_shift_round(integ, SSTEP_INTEG_FRAC_BITS - SSTEP_REAL_FRAC_BITS);
}

/**
 * @brief  num / den for 0 <= num <= den, den > 0, as a signal in 0 .. +0.99997
 *         (rounded, 1.0 saturates). One 32-bit division.
 */
static inline sstep_real_t sstep_div_unit(sstep_acc_t num, sstep_acc_t den)
{
    uint32_t n = (uint32_t)num;
    uint32_t d = (uint32_t)den;
    uint32_t r;
    while (d >= 65536U) /* at most 16 iterations; keeps n << 15 below 2^31 */
    {
        n >>= 1U;
        d >>= 1U;
    }
    r = ((n << 15U) + (d >> 1U)) / d;
    if (r > 32767U)
    {
        r = 32767U;
    }
    return (sstep_real_t)r;
}

/** @brief x * k for |x| < 2 pu (saturated first), rounded. */
static inline sstep_acc_t sstep_acc_scale(sstep_acc_t x, sstep_real_t k)
{
    int32_t xc = sstep_acc_clamp(x, -SSTEP_ACC_SCALE_MAX, SSTEP_ACC_SCALE_MAX);
    return sstep_shift_round(xc * (int32_t)k, 15); /* |xc * k| < 2^31 */
}

/** @brief Duty cycle (0 .. 1, clamped) to PWM compare ticks, rounded. */
static inline uint16_t sstep_acc_to_ticks(sstep_acc_t duty, uint16_t period)
{
    uint32_t d = (uint32_t)sstep_acc_clamp(duty, 0, SSTEP_ACC_ONE);
    return (uint16_t)(((d * (uint32_t)period) + 16384U) >> 15U);
}

/* ==============================================================================
 * FLOAT OPERATIONS
 * =============================================================================*/

#else /* SSTEP_MATH_BACKEND_FLOAT */

/** @brief Limit a float to the ::sstep_real_t range. */
static inline sstep_real_t sstep_sat_f(float x)
{
    float r = x;
    if (r > SSTEP_REAL_MAX)
    {
        r = SSTEP_REAL_MAX;
    }
    else if (r < SSTEP_REAL_MIN)
    {
        r = SSTEP_REAL_MIN;
    }
    else
    {
        /* in range */
    }
    return r;
}

/** @brief Saturating a + b. */
static inline sstep_real_t sstep_add(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a + b);
}

/** @brief Saturating a - b. */
static inline sstep_real_t sstep_sub(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a - b);
}

/** @brief Saturating -a. */
static inline sstep_real_t sstep_neg(sstep_real_t a)
{
    return sstep_sat_f(-a);
}

/** @brief Saturating |a|. */
static inline sstep_real_t sstep_abs(sstep_real_t a)
{
    return sstep_sat_f((a < 0.0f) ? -a : a);
}

/** @brief a * b, saturating. */
static inline sstep_real_t sstep_mul(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a * b);
}

/** @brief Convert a signal to an accumulator (exact). */
static inline sstep_acc_t sstep_acc_from_real(sstep_real_t a)
{
    return a;
}

/** @brief Convert an accumulator to a signal, saturating. */
static inline sstep_real_t sstep_acc_to_real(sstep_acc_t a)
{
    return sstep_sat_f(a);
}

/** @brief Accumulator limited to lo .. hi (requires lo <= hi). */
static inline sstep_acc_t sstep_acc_clamp(sstep_acc_t x, sstep_acc_t lo, sstep_acc_t hi)
{
    sstep_acc_t r = x;
    if (r > hi)
    {
        r = hi;
    }
    else if (r < lo)
    {
        r = lo;
    }
    else
    {
        /* in range */
    }
    return r;
}

/** @brief Saturating accumulator a + b. */
static inline sstep_acc_t sstep_acc_add(sstep_acc_t a, sstep_acc_t b)
{
    return sstep_acc_clamp(a + b, SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

/** @brief Gain times signal. */
static inline sstep_acc_t sstep_gain_mul(const struct sstep_gain* g, sstep_real_t x)
{
    return sstep_acc_clamp(g->value * x, SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

/** @brief Gain times integer (n saturated to -32768 .. 32767 as in the fixed backend). */
static inline sstep_acc_t sstep_gain_mul_int(const struct sstep_gain* g, int32_t n)
{
    int32_t k = n;
    if (k > (int32_t)INT16_MAX)
    {
        k = (int32_t)INT16_MAX;
    }
    else if (k < (int32_t)INT16_MIN)
    {
        k = (int32_t)INT16_MIN;
    }
    else
    {
        /* in range */
    }
    return sstep_acc_clamp(g->value * (float)k, SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}


/** @brief Saturating |a| of an accumulator. */
static inline sstep_acc_t sstep_acc_abs(sstep_acc_t a)
{
    return (a < 0.0f) ? -a : a;
}

/** @brief a / 2. */
static inline sstep_acc_t sstep_acc_halve(sstep_acc_t a)
{
    return a * 0.5f;
}

/** @brief Limit an accumulator to +/-::SSTEP_ACC_IN_MAX. */
static inline sstep_acc_t sstep_acc_in(sstep_acc_t x)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_IN_MAX, SSTEP_ACC_IN_MAX);
}

/** @brief Gain times accumulator, |x| < 4 pu (saturated first). */
static inline sstep_acc_t sstep_gain_mul_acc(const struct sstep_gain* g, sstep_acc_t x)
{
    return sstep_acc_clamp(g->value * sstep_acc_in(x), SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

/** @brief Integrator + g * x (x a signal), saturating to +/-16 pu. */
static inline sstep_integ_t sstep_integ_add(sstep_integ_t integ, const struct sstep_gain* g,
                                            sstep_real_t x)
{
    return sstep_acc_clamp(integ + (g->value * x), -16.0f, 15.999999f);
}

/** @brief Integrator + g * x (x an accumulator, |x| < 4 pu), saturating to +/-16 pu. */
static inline sstep_integ_t sstep_integ_add_acc(sstep_integ_t integ, const struct sstep_gain* g,
                                                sstep_acc_t x)
{
    return sstep_acc_clamp(integ + (g->value * sstep_acc_in(x)), -16.0f, 15.999999f);
}

/** @brief Integrator value as an accumulator. */
static inline sstep_acc_t sstep_integ_to_acc(sstep_integ_t integ)
{
    return integ;
}

/** @brief num / den for 0 <= num <= den, den > 0, limited to 0 .. +0.99997. */
static inline sstep_real_t sstep_div_unit(sstep_acc_t num, sstep_acc_t den)
{
    return sstep_sat_f(num / den);
}

/** @brief x * k for |x| < 2 pu (saturated first). */
static inline sstep_acc_t sstep_acc_scale(sstep_acc_t x, sstep_real_t k)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_SCALE_MAX, SSTEP_ACC_SCALE_MAX) * k;
}

/** @brief Duty cycle (0 .. 1, clamped) to PWM compare ticks, rounded. */
static inline uint16_t sstep_acc_to_ticks(sstep_acc_t duty, uint16_t period)
{
    float d = sstep_acc_clamp(duty, 0.0f, 1.0f);
    return (uint16_t)((d * (float)period) + 0.5f);
}

#endif /* SSTEP_CFG_MATH_BACKEND */

/* ==============================================================================
 * COMMON OPERATIONS
 * =============================================================================*/

/** @brief x limited to lo .. hi (requires lo <= hi). */
static inline sstep_real_t sstep_clamp(sstep_real_t x, sstep_real_t lo, sstep_real_t hi)
{
    sstep_real_t r = x;
    if (r > hi)
    {
        r = hi;
    }
    else if (r < lo)
    {
        r = lo;
    }
    else
    {
        /* in range */
    }
    return r;
}

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * =============================================================================*/

/**
 * @brief  Sine and cosine of an electrical angle from a quarter-wave table
 *         (::SSTEP_CFG_SINE_TABLE_BITS) with linear interpolation. Fast loop safe.
 *         Quadrant symmetric: sin(a + 90 deg) is bit-exactly cos(a).
 * @param  angle  Angle, 2^32 = one turn.
 * @return Sine and cosine.
 */
struct sstep_sincos sstep_sin_cos(sstep_angle_t angle);

/**
 * @brief  Create a gain equal to num / den. Initialisation only (64-bit division).
 * @param  num   Numerator.
 * @param  den   Denominator, not 0.
 * @param  gain  Result.
 * @retval ::SSTEP_OK         Success.
 * @retval ::SSTEP_ERR_PARAM  gain is NULL, den is 0, or |num / den| rounds above 32767.
 * @note   Every non-zero int32 ratio is at least 2^-31 and is representable; ratios
 *         below 2^-16 lose resolution (the mantissa is no longer normalised).
 */
int sstep_gain_from_ratio(int32_t num, int32_t den, struct sstep_gain* gain);

/**
 * @brief  Create a gain equal to num / den with 64-bit operands, for gains computed from
 *         several engineering values (for example L * wc * I_base / V_base).
 *         Initialisation and slow loop only: at most three 64-bit divisions.
 * @param  num   Numerator, |num| < 2^62.
 * @param  den   Denominator, 0 < |den| < 2^62.
 * @param  gain  Result.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (gain NULL, den 0, an operand out of range, or
 *         |num / den| rounds above 32767). Ratios below 2^-31 give 0 (fixed point).
 */
int sstep_gain_from_ratio64(int64_t num, int64_t den, struct sstep_gain* gain);

/**
 * @brief  Convert num / den to a per-unit signal, rounded and saturated. Initialisation
 *         and slow loop only (64-bit division).
 * @param  num  Numerator, for example 1500 mA.
 * @param  den  Denominator, the base, for example 4000 mA; not 0.
 * @param  out  Result.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (out NULL or den 0).
 */
int sstep_real_from_ratio(int32_t num, int32_t den, sstep_real_t* out);

/**
 * @brief  Convert a per-unit value back to engineering units: round(x * base).
 *         Slow loop and status only.
 * @param  x     Per-unit accumulator.
 * @param  base  Base in engineering units, for example 4000 mA.
 * @return x * base, rounded to nearest and saturated to int32_t.
 */
int32_t sstep_real_to_units(sstep_acc_t x, int32_t base);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SSTEP_MATH_H */
