

# File sstep\_math.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_math.h**](sstep__math_8h.md)

[Go to the documentation of this file](sstep__math_8h.md)


```C++
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

typedef uint32_t sstep_angle_t;

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

typedef int16_t sstep_real_t;

typedef int32_t sstep_acc_t;

struct sstep_gain
{
    int16_t mant; 
    int8_t exp;   
};

#define SSTEP_REAL_FRAC_BITS 15

#define SSTEP_REAL_MAX ((sstep_real_t)INT16_MAX)

#define SSTEP_REAL_MIN ((sstep_real_t)INT16_MIN)

#define SSTEP_ACC_MAX ((sstep_acc_t)INT32_MAX)

#define SSTEP_ACC_MIN ((sstep_acc_t)INT32_MIN)

#define SSTEP_ACC_C(x) ((sstep_acc_t)((x) * 32768.0))

typedef int32_t sstep_integ_t;

#define SSTEP_INTEG_FRAC_BITS 27

#define SSTEP_REAL_C(x)                                                                 \
    (((x) >= 1.0) ? SSTEP_REAL_MAX : (((x) <= -1.0) ? SSTEP_REAL_MIN                     \
                                                    : (sstep_real_t)((x) * 32768.0)))

#else /* SSTEP_MATH_BACKEND_FLOAT */

typedef float sstep_real_t;

typedef float sstep_acc_t;

struct sstep_gain
{
    float value; 
};

#define SSTEP_REAL_MAX (0.999969482421875f)

#define SSTEP_REAL_MIN (-1.0f)

#define SSTEP_ACC_MAX (65535.99609375f)

#define SSTEP_ACC_MIN (-65536.0f)

#define SSTEP_ACC_C(x) ((sstep_acc_t)(x))

typedef float sstep_integ_t;

#define SSTEP_REAL_C(x)                                                                 \
    (((x) >= 1.0) ? SSTEP_REAL_MAX : (((x) <= -1.0) ? SSTEP_REAL_MIN : (sstep_real_t)(x)))

#endif /* SSTEP_CFG_MATH_BACKEND */

#define SSTEP_REAL_ZERO SSTEP_REAL_C(0.0)

#define SSTEP_ACC_ONE SSTEP_ACC_C(1.0)

#define SSTEP_ACC_IN_MAX SSTEP_ACC_C(3.999969482421875)

#define SSTEP_ACC_SCALE_MAX SSTEP_ACC_C(1.999969482421875)

#define SSTEP_GAIN_EXP_MIN (-16)

#define SSTEP_GAIN_EXP_MAX 15

struct sstep_sincos
{
    sstep_real_t sin; 
    sstep_real_t cos; 
};

/* ==============================================================================
 * FIXED POINT OPERATIONS
 * =============================================================================*/

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

static inline int32_t sstep_asr(int32_t x, uint32_t n)
{
    int32_t result;
    if (x >= 0)
    {
        result = (int32_t)((uint32_t)x >> n);
    }
    else
    {
        result = -(int32_t)((uint32_t)(-(x + 1)) >> n) - 1;
    }
    return result;
}

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

static inline sstep_real_t sstep_add(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat((int32_t)a + (int32_t)b);
}

static inline sstep_real_t sstep_sub(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat((int32_t)a - (int32_t)b);
}

static inline sstep_real_t sstep_neg(sstep_real_t a)
{
    return sstep_sat(-(int32_t)a);
}

static inline sstep_real_t sstep_abs(sstep_real_t a)
{
    return sstep_sat((a < 0) ? -(int32_t)a : (int32_t)a);
}

static inline sstep_real_t sstep_mul(sstep_real_t a, sstep_real_t b)
{
    int32_t p = (int32_t)a * (int32_t)b; /* |p| <= 2^30: no overflow */
    return sstep_sat(sstep_asr(p + 0x4000, 15U));
}

static inline sstep_acc_t sstep_acc_from_real(sstep_real_t a)
{
    return (sstep_acc_t)a;
}

static inline sstep_real_t sstep_acc_to_real(sstep_acc_t a)
{
    return sstep_sat(a);
}

static inline sstep_acc_t sstep_acc_add(sstep_acc_t a, sstep_acc_t b)
{
    sstep_acc_t r;
    if ((b > 0) && (a > (INT32_MAX - b)))
    {
        r = INT32_MAX;
    }
    else if ((b < 0) && (a < (INT32_MIN - b)))
    {
        r = INT32_MIN;
    }
    else
    {
        r = a + b;
    }
    return r;
}

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

static inline sstep_acc_t sstep_gain_mul(const struct sstep_gain* g, sstep_real_t x)
{
    int32_t p = (int32_t)x * (int32_t)g->mant;                 /* Q30, |p| <= 2^30 */
    uint32_t sh = (uint32_t)(15 - (int32_t)g->exp);            /* 0 .. 31 */
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
            r = INT32_MIN;
        }
        else
        {
            r = p * (int32_t)((uint32_t)1U << (uint32_t)e); /* |p| <= lim: no overflow */
        }
    }
    else
    {
        uint32_t sh = (uint32_t)(-e); /* 1 .. 16 */
        r = sstep_asr(sstep_asr(p, sh - 1U) + 1, 1U);
    }
    return r;
}


static inline int32_t sstep_shift_round(int32_t p, int32_t sh)
{
    int32_t r;
    if (sh > 0)
    {
        r = sstep_asr(sstep_asr(p, (uint32_t)(sh - 1)) + 1, 1U);
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
            r = INT32_MIN;
        }
        else
        {
            r = p * (int32_t)((uint32_t)1U << n); /* |p| <= lim: no overflow */
        }
    }
    return r;
}

static inline sstep_acc_t sstep_acc_abs(sstep_acc_t a)
{
    return (a < 0) ? ((a == INT32_MIN) ? INT32_MAX : -a) : a;
}

static inline sstep_acc_t sstep_acc_halve(sstep_acc_t a)
{
    return sstep_asr(a, 1U);
}

static inline sstep_acc_t sstep_acc_in(sstep_acc_t x)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_IN_MAX, SSTEP_ACC_IN_MAX);
}

static inline sstep_acc_t sstep_gain_mul_acc(const struct sstep_gain* g, sstep_acc_t x)
{
    int32_t p = sstep_asr(sstep_acc_in(x), 2U) * (int32_t)g->mant; /* |p| < 2^30 */
    return sstep_shift_round(p, 13 - (int32_t)g->exp);
}

static inline sstep_integ_t sstep_integ_add(sstep_integ_t integ, const struct sstep_gain* g,
                                            sstep_real_t x)
{
    int32_t p = (int32_t)x * (int32_t)g->mant; /* Q30, |p| <= 2^30 */
    return sstep_acc_add(integ, sstep_shift_round(p, 3 - (int32_t)g->exp));
}

static inline sstep_integ_t sstep_integ_add_acc(sstep_integ_t integ, const struct sstep_gain* g,
                                                sstep_acc_t x)
{
    int32_t p = sstep_asr(sstep_acc_in(x), 2U) * (int32_t)g->mant;
    return sstep_acc_add(integ, sstep_shift_round(p, 1 - (int32_t)g->exp));
}

static inline sstep_acc_t sstep_integ_to_acc(sstep_integ_t integ)
{
    return sstep_shift_round(integ, SSTEP_INTEG_FRAC_BITS - SSTEP_REAL_FRAC_BITS);
}

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

static inline sstep_acc_t sstep_acc_scale(sstep_acc_t x, sstep_real_t k)
{
    int32_t xc = sstep_acc_clamp(x, -SSTEP_ACC_SCALE_MAX, SSTEP_ACC_SCALE_MAX);
    return sstep_shift_round(xc * (int32_t)k, 15); /* |xc * k| < 2^31 */
}

static inline uint16_t sstep_acc_to_ticks(sstep_acc_t duty, uint16_t period)
{
    uint32_t d = (uint32_t)sstep_acc_clamp(duty, 0, SSTEP_ACC_ONE);
    return (uint16_t)(((d * (uint32_t)period) + 16384U) >> 15U);
}

/* ==============================================================================
 * FLOAT OPERATIONS
 * =============================================================================*/

#else /* SSTEP_MATH_BACKEND_FLOAT */

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

static inline sstep_real_t sstep_add(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a + b);
}

static inline sstep_real_t sstep_sub(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a - b);
}

static inline sstep_real_t sstep_neg(sstep_real_t a)
{
    return sstep_sat_f(-a);
}

static inline sstep_real_t sstep_abs(sstep_real_t a)
{
    return sstep_sat_f((a < 0.0f) ? -a : a);
}

static inline sstep_real_t sstep_mul(sstep_real_t a, sstep_real_t b)
{
    return sstep_sat_f(a * b);
}

static inline sstep_acc_t sstep_acc_from_real(sstep_real_t a)
{
    return a;
}

static inline sstep_real_t sstep_acc_to_real(sstep_acc_t a)
{
    return sstep_sat_f(a);
}

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

static inline sstep_acc_t sstep_acc_add(sstep_acc_t a, sstep_acc_t b)
{
    return sstep_acc_clamp(a + b, SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

static inline sstep_acc_t sstep_gain_mul(const struct sstep_gain* g, sstep_real_t x)
{
    return sstep_acc_clamp(g->value * x, SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

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


static inline sstep_acc_t sstep_acc_abs(sstep_acc_t a)
{
    return (a < 0.0f) ? -a : a;
}

static inline sstep_acc_t sstep_acc_halve(sstep_acc_t a)
{
    return a * 0.5f;
}

static inline sstep_acc_t sstep_acc_in(sstep_acc_t x)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_IN_MAX, SSTEP_ACC_IN_MAX);
}

static inline sstep_acc_t sstep_gain_mul_acc(const struct sstep_gain* g, sstep_acc_t x)
{
    return sstep_acc_clamp(g->value * sstep_acc_in(x), SSTEP_ACC_MIN, SSTEP_ACC_MAX);
}

static inline sstep_integ_t sstep_integ_add(sstep_integ_t integ, const struct sstep_gain* g,
                                            sstep_real_t x)
{
    return sstep_acc_clamp(integ + (g->value * x), -16.0f, 15.999999f);
}

static inline sstep_integ_t sstep_integ_add_acc(sstep_integ_t integ, const struct sstep_gain* g,
                                                sstep_acc_t x)
{
    return sstep_acc_clamp(integ + (g->value * sstep_acc_in(x)), -16.0f, 15.999999f);
}

static inline sstep_acc_t sstep_integ_to_acc(sstep_integ_t integ)
{
    return integ;
}

static inline sstep_real_t sstep_div_unit(sstep_acc_t num, sstep_acc_t den)
{
    return sstep_sat_f(num / den);
}

static inline sstep_acc_t sstep_acc_scale(sstep_acc_t x, sstep_real_t k)
{
    return sstep_acc_clamp(x, -SSTEP_ACC_SCALE_MAX, SSTEP_ACC_SCALE_MAX) * k;
}

static inline uint16_t sstep_acc_to_ticks(sstep_acc_t duty, uint16_t period)
{
    float d = sstep_acc_clamp(duty, 0.0f, 1.0f);
    return (uint16_t)((d * (float)period) + 0.5f);
}

#endif /* SSTEP_CFG_MATH_BACKEND */

/* ==============================================================================
 * COMMON OPERATIONS
 * =============================================================================*/

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

struct sstep_sincos sstep_sin_cos(sstep_angle_t angle);

int sstep_gain_from_ratio(int32_t num, int32_t den, struct sstep_gain* gain);

int sstep_gain_from_ratio64(int64_t num, int64_t den, struct sstep_gain* gain);

int sstep_real_from_ratio(int32_t num, int32_t den, sstep_real_t* out);

int32_t sstep_real_to_units(sstep_acc_t x, int32_t base);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_MATH_H */
```


