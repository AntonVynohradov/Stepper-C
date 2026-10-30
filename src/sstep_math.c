/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Numeric backend: sine/cosine and conversions from engineering units.
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

#include "sstep_math.h"

#include <stdbool.h>
#include <stddef.h>

#include "sstep_defs.h"
#include "sstep_sin_table.h"

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** Bits of an angle inside one quadrant. */
#define QUADRANT_BITS 30U

/** Mask of the angle inside one quadrant. */
#define QUADRANT_MASK 0x3FFFFFFFUL

/** Bits of the angle below the table index (interpolation fraction). */
#define FRAC_BITS (QUADRANT_BITS - SSTEP_CFG_SINE_TABLE_BITS)

/** Operand limit of sstep_gain_from_ratio64(): 2^62. */
#define RATIO64_LIMIT ((int64_t)1 << 62)

/* ==============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static sstep_real_t quarter_sin(uint32_t pos);

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
static uint32_t bit_length(uint64_t v);
static uint64_t ratio_round(uint64_t n, uint64_t d, uint32_t s);
#endif

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

/** @brief Number of significant bits of v (0 for 0); binary search, 6 steps. */
static uint32_t bit_length(uint64_t v)
{
    uint64_t x = v;
    uint32_t n = 0U;
    uint32_t step = 32U;
    while (step > 0U)
    {
        if ((x >> step) != 0U)
        {
            x >>= step;
            n += step;
        }
        step >>= 1U;
    }
    return n + (uint32_t)x; /* x is 0 or 1 here */
}

/** @brief round(n * 2^s / d); the caller guarantees n * 2^s < 2^63. */
static uint64_t ratio_round(uint64_t n, uint64_t d, uint32_t s)
{
    return ((n << s) + (d / 2U)) / d;
}

/**
 * @brief  sin(pos) for 0 <= pos < 90 deg (pos in 2^-32 turn), interpolated.
 *         The index is at most N - 1, so idx + 1 is inside the table (N + 1 entries).
 */
static sstep_real_t quarter_sin(uint32_t pos)
{
    uint32_t idx = pos >> FRAC_BITS;
    /* the 15 most significant bits of the fraction (FRAC_BITS >= 20) */
    int32_t frac = (int32_t)((pos >> (FRAC_BITS - 15U)) & 0x7FFFUL);
    int32_t y0 = (int32_t)sstep_sin_table[idx];
    int32_t dy = (int32_t)sstep_sin_table[idx + 1U] - y0; /* |dy| < 2^10 */
    return (sstep_real_t)(y0 + sstep_asr((dy * frac) + 0x4000, 15U));
}

#else /* SSTEP_MATH_BACKEND_FLOAT */

/** Scale of the interpolation fraction: 2^-FRAC_BITS. */
#define FRAC_SCALE (1.0f / (float)(1UL << FRAC_BITS))

/** @brief sin(pos) for 0 <= pos < 90 deg, interpolated. */
static sstep_real_t quarter_sin(uint32_t pos)
{
    uint32_t idx = pos >> FRAC_BITS;
    float frac = (float)(pos & ((1UL << FRAC_BITS) - 1UL)) * FRAC_SCALE;
    float y0 = sstep_sin_table[idx];
    return y0 + ((sstep_sin_table[idx + 1U] - y0) * frac);
}

#endif /* SSTEP_CFG_MATH_BACKEND */

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

struct sstep_sincos sstep_sin_cos(sstep_angle_t angle)
{
    uint32_t quadrant = angle >> QUADRANT_BITS;
    uint32_t pos = angle & QUADRANT_MASK;
    /* cos(pos) = sin(90 deg - pos); one angle LSB (2^-32 turn) less keeps the index
       below N, and makes sin(a + 90 deg) bit-exactly equal to cos(a). */
    sstep_real_t s = quarter_sin(pos);
    sstep_real_t c = quarter_sin(QUADRANT_MASK - pos);
    struct sstep_sincos r;

    switch (quadrant)
    {
    case 0U:
        r.sin = s;
        r.cos = c;
        break;
    case 1U:
        r.sin = c;
        r.cos = sstep_neg(s);
        break;
    case 2U:
        r.sin = sstep_neg(s);
        r.cos = sstep_neg(c);
        break;
    default: /* quadrant 3 */
        r.sin = sstep_neg(c);
        r.cos = s;
        break;
    }
    return r;
}

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)

int sstep_gain_from_ratio64(int64_t num, int64_t den, struct sstep_gain* gain)
{
    int result = SSTEP_OK;

    if ((gain == NULL) || (den == 0) || (num >= RATIO64_LIMIT) || (num <= -RATIO64_LIMIT) ||
        (den >= RATIO64_LIMIT) || (den <= -RATIO64_LIMIT))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        bool negative = ((num < 0) != (den < 0));
        uint64_t n = (num < 0) ? (uint64_t)(-num) : (uint64_t)num;
        uint64_t d = (den < 0) ? (uint64_t)(-den) : (uint64_t)den;
        uint32_t ld = bit_length(d);
        int32_t s;
        uint64_t m;

        if (ld > 47U)
        {
            /* keep n << s below 2^63; the relative error of d is below 2^-46 */
            n >>= (ld - 47U);
            d >>= (ld - 47U);
            ld = 47U;
        }

        /* estimate: n * 2^s / d in (2^13, 2^15], then correct by one step */
        s = 14 - (int32_t)bit_length(n) + (int32_t)ld;
        if (s < 0)
        {
            s = 0;
        }
        else if (s > 31)
        {
            s = 31;
        }
        else
        {
            /* in range */
        }
        m = ratio_round(n, d, (uint32_t)s);

        if (m > 32767U)
        {
            if (s == 0)
            {
                result = SSTEP_ERR_PARAM; /* |num / den| above 32767 */
            }
            else
            {
                s--;
                m = ratio_round(n, d, (uint32_t)s);
            }
        }
        else if (s < 31)
        {
            uint64_t m2 = ratio_round(n, d, (uint32_t)s + 1U);
            if (m2 <= 32767U)
            {
                s++;
                m = m2;
            }
        }
        else
        {
            /* s = 31: smallest exponent */
        }

        if (result == SSTEP_OK)
        {
            if (m == 0U)
            {
                gain->mant = 0;
                gain->exp = (int8_t)SSTEP_GAIN_EXP_MIN;
            }
            else
            {
                int32_t mi = (int32_t)m;
                gain->mant = (int16_t)(negative ? -mi : mi);
                gain->exp = (int8_t)(15 - s);
            }
        }
    }
    return result;
}

int sstep_gain_from_ratio(int32_t num, int32_t den, struct sstep_gain* gain)
{
    return sstep_gain_from_ratio64((int64_t)num, (int64_t)den, gain);
}

int sstep_real_from_ratio(int32_t num, int32_t den, sstep_real_t* out)
{
    int result = SSTEP_OK;

    if ((out == NULL) || (den == 0))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        bool negative = ((num < 0) != (den < 0));
        uint64_t n = (num < 0) ? (uint64_t)(-(int64_t)num) : (uint64_t)num;
        uint64_t d = (den < 0) ? (uint64_t)(-(int64_t)den) : (uint64_t)den;
        uint64_t m = ((n << 15U) + (d / 2U)) / d;
        int32_t v;
        if (m > 32768U)
        {
            m = 32768U; /* saturate before the sign is applied */
        }
        v = (int32_t)m;
        *out = sstep_sat(negative ? -v : v);
    }
    return result;
}

int32_t sstep_real_to_units(sstep_acc_t x, int32_t base)
{
    int64_t p = (int64_t)x * (int64_t)base;
    uint64_t a = (p < 0) ? (uint64_t)(-p) : (uint64_t)p; /* |p| < 2^62 */
    uint64_t m = (a + 0x4000U) >> 15U;
    int64_t v = (p < 0) ? -(int64_t)m : (int64_t)m;
    int32_t result;

    if (v > (int64_t)INT32_MAX)
    {
        result = INT32_MAX;
    }
    else if (v < (int64_t)INT32_MIN)
    {
        result = INT32_MIN;
    }
    else
    {
        result = (int32_t)v;
    }
    return result;
}

#else /* SSTEP_MATH_BACKEND_FLOAT */

int sstep_gain_from_ratio64(int64_t num, int64_t den, struct sstep_gain* gain)
{
    int result = SSTEP_OK;

    if ((gain == NULL) || (den == 0) || (num >= RATIO64_LIMIT) || (num <= -RATIO64_LIMIT) ||
        (den >= RATIO64_LIMIT) || (den <= -RATIO64_LIMIT))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        float v = (float)num / (float)den;
        float a = (v < 0.0f) ? -v : v;
        /* same limit as the fixed point backend (round(|v|) <= 32767) */
        if (a >= 32767.5f)
        {
            result = SSTEP_ERR_PARAM;
        }
        else
        {
            gain->value = v;
        }
    }
    return result;
}

int sstep_gain_from_ratio(int32_t num, int32_t den, struct sstep_gain* gain)
{
    return sstep_gain_from_ratio64((int64_t)num, (int64_t)den, gain);
}

int sstep_real_from_ratio(int32_t num, int32_t den, sstep_real_t* out)
{
    int result = SSTEP_OK;

    if ((out == NULL) || (den == 0))
    {
        result = SSTEP_ERR_PARAM;
    }
    else
    {
        *out = sstep_sat_f((float)num / (float)den);
    }
    return result;
}

int32_t sstep_real_to_units(sstep_acc_t x, int32_t base)
{
    float v = x * (float)base;
    int32_t result;

    if (v >= 2147483520.0f) /* largest float below 2^31 */
    {
        result = INT32_MAX;
    }
    else if (v <= -2147483648.0f)
    {
        result = INT32_MIN;
    }
    else
    {
        result = (int32_t)((v < 0.0f) ? (v - 0.5f) : (v + 0.5f));
    }
    return result;
}

#endif /* SSTEP_CFG_MATH_BACKEND */
