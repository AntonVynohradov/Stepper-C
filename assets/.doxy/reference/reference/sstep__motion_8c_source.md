

# File sstep\_motion.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_motion.c**](sstep__motion_8c.md)

[Go to the documentation of this file](sstep__motion_8c.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Motion generator: trapezoidal velocity profile in integer arithmetic (slow loop).
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
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi);
static int32_t abs_i32(int32_t v);
static int64_t abs_i64(int64_t v);
static int64_t half_floor(int64_t v);
static int32_t target_velocity(const struct sstep_motion* mo);

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    int32_t r = v;
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

static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

static int64_t abs_i64(int64_t v)
{
    return (v < 0) ? -v : v;
}

static int64_t half_floor(int64_t v)
{
    return (v >= 0) ? (v / 2) : -((1 - v) / 2);
}

static int32_t target_velocity(const struct sstep_motion* mo)
{
    int32_t vt;

    if (mo->mode == SSTEP_MODE_MOVE)
    {
        int64_t rem = mo->target - mo->pos;
        int64_t arem = abs_i64(rem);
        int64_t av = (int64_t)abs_i32(mo->vel);
        int64_t d = (int64_t)mo->dec;
        uint32_t mag;

        if (arem >= (mo->brake_vmax + (((int64_t)mo->v_max + av) / 2)))
        {
            mag = (uint32_t)mo->v_max;
        }
        else
        {
            /* arem < v_max^2 / (2 dec) + v_max: every term below 2^62. The root is below
               v_max here (it reaches v_max exactly at the threshold above). */
            int64_t disc = (d * d) + (8 * d * arem) - (4 * d * av);
            uint32_t root = (disc > 0) ? sstep_isqrt64((uint64_t)disc) : 0U;
            mag = (root > (uint32_t)d) ? ((root - (uint32_t)d) / 2U) : 0U;
        }

        if (rem > 0)
        {
            vt = (int32_t)mag;
        }
        else if (rem < 0)
        {
            vt = -(int32_t)mag;
        }
        else
        {
            vt = 0;
        }
    }
    else if (mo->mode == SSTEP_MODE_RUN)
    {
        vt = clamp_i32(mo->v_run, -mo->v_max, mo->v_max);
    }
    else
    {
        vt = 0;
    }
    return vt;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS (library internal)
 * =============================================================================*/

uint32_t sstep_isqrt64(uint64_t x)
{
    uint64_t rem = x;
    uint64_t res = 0U;
    uint64_t bit = (uint64_t)1U << 62U;

    while (bit > rem)
    {
        bit >>= 2U;
    }
    while (bit != 0U)
    {
        if (rem >= (res + bit))
        {
            rem -= res + bit;
            res = (res >> 1U) + bit;
        }
        else
        {
            res >>= 1U;
        }
        bit >>= 2U;
    }
    return (uint32_t)res;
}

uint8_t sstep_motion_step(struct sstep_motion* mo)
{
    uint8_t result = 0U;
    int32_t vt = target_velocity(mo);
    /* speeding up: larger magnitude in the same direction (or from standstill) */
    bool speeding_up =
        (abs_i32(vt) > abs_i32(mo->vel)) && ((mo->vel == 0) || ((vt > 0) == (mo->vel > 0)));
    int32_t a = speeding_up ? mo->acc : mo->dec;
    int32_t v_new = mo->vel + clamp_i32(vt - mo->vel, -a, a);
    int64_t p_new = mo->pos + half_floor((int64_t)mo->vel + (int64_t)v_new);

    if (mo->mode == SSTEP_MODE_MOVE)
    {
        int64_t rem0 = mo->target - mo->pos;
        int64_t rem1 = mo->target - p_new;
        bool crossed = ((rem0 >= 0) && (rem1 <= 0)) || ((rem0 <= 0) && (rem1 >= 0));
        bool near = (abs_i64(rem1) < SSTEP_POS_SNAP) && (abs_i32(v_new) <= mo->dec);
        if (crossed || near)
        {
            p_new = mo->target;
            v_new = 0;
            mo->mode = SSTEP_MODE_HOLD;
            result = SSTEP_MOTION_DONE;
        }
    }

    mo->pos = p_new;
    mo->vel = v_new;
    return result;
}
```


