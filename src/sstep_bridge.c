/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Averaged power stage: phase voltage request to leg compare values, both topologies.
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

/** 0.5 (50 % duty cycle). */
#define ACC_HALF SSTEP_ACC_C(0.5)

/* ==============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * =============================================================================*/

static sstep_acc_t acc_max(sstep_acc_t a, sstep_acc_t b);
#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)
static sstep_acc_t acc_min(sstep_acc_t a, sstep_acc_t b);
#endif
static sstep_acc_t scale_to_unit(sstep_acc_t x, sstep_acc_t m);
#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1)
static sstep_acc_t span_full(sstep_acc_t a, sstep_acc_t b);
static void duties_full(sstep_acc_t a, sstep_acc_t b, uint16_t period, struct sstep_bridge_out* out);
#endif
#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)
static sstep_acc_t span_shared(sstep_acc_t a, sstep_acc_t b);
static void duties_shared(sstep_acc_t a, sstep_acc_t b, uint16_t period,
                          struct sstep_bridge_out* out);
#endif

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

/** @brief Larger of a and b. */
static sstep_acc_t acc_max(sstep_acc_t a, sstep_acc_t b)
{
    return (a > b) ? a : b;
}

#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)
/** @brief Smaller of a and b. */
static sstep_acc_t acc_min(sstep_acc_t a, sstep_acc_t b)
{
    return (a < b) ? a : b;
}
#endif

/** @brief x / m for |x| <= m, keeping the sign of x (one division). */
static sstep_acc_t scale_to_unit(sstep_acc_t x, sstep_acc_t m)
{
    sstep_acc_t q = sstep_acc_from_real(sstep_div_unit(sstep_acc_abs(x), m));
    return (x < SSTEP_ACC_C(0.0)) ? -q : q;
}

#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1)

/** @brief Full bridge: the larger phase voltage relative to Vbus (limit 1). */
static sstep_acc_t span_full(sstep_acc_t a, sstep_acc_t b)
{
    return acc_max(sstep_acc_abs(a), sstep_acc_abs(b));
}

/** @brief Full bridge: legs [A+ A- B+ B-] centred at 50 %. */
static void duties_full(sstep_acc_t a, sstep_acc_t b, uint16_t period, struct sstep_bridge_out* out)
{
    sstep_acc_t ha = sstep_acc_halve(a);
    sstep_acc_t hb = sstep_acc_halve(b);
    out->compare[0] = sstep_acc_to_ticks(ACC_HALF + ha, period);
    out->compare[1] = sstep_acc_to_ticks(ACC_HALF - ha, period);
    out->compare[2] = sstep_acc_to_ticks(ACC_HALF + hb, period);
    out->compare[3] = sstep_acc_to_ticks(ACC_HALF - hb, period);
}

#endif

#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)

/** @brief Shared leg: max(a, b, 0) - min(a, b, 0) relative to Vbus (limit 1). */
static sstep_acc_t span_shared(sstep_acc_t a, sstep_acc_t b)
{
    return acc_max(acc_max(a, b), SSTEP_ACC_C(0.0)) - acc_min(acc_min(a, b), SSTEP_ACC_C(0.0));
}

/** @brief Shared leg: legs [U V W], A = U - W, B = V - W, common leg W centred between
 *         the highest and the lowest leg voltage. */
static void duties_shared(sstep_acc_t a, sstep_acc_t b, uint16_t period,
                          struct sstep_bridge_out* out)
{
    sstep_acc_t hi = acc_max(acc_max(a, b), SSTEP_ACC_C(0.0));
    sstep_acc_t lo = acc_min(acc_min(a, b), SSTEP_ACC_C(0.0));
    sstep_acc_t w = ACC_HALF - sstep_acc_halve(hi + lo);
    out->compare[0] = sstep_acc_to_ticks(a + w, period);
    out->compare[1] = sstep_acc_to_ticks(b + w, period);
    out->compare[2] = sstep_acc_to_ticks(w, period);
    out->compare[3] = 0U;
}

#endif

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS (library internal)
 * =============================================================================*/

void sstep_bridge_map(enum sstep_topology topology, sstep_acc_t va, sstep_acc_t vb,
                      const struct sstep_gain* inv_vbus, sstep_acc_t vbus, uint16_t period,
                      struct sstep_bridge_out* out)
{
    sstep_acc_t x = va;
    sstep_acc_t y = vb;
    sstep_acc_t m0 = acc_max(sstep_acc_abs(va), sstep_acc_abs(vb));
    sstep_acc_t a;
    sstep_acc_t b;
    sstep_acc_t m;

    /* Power-of-two pre-scale keeps the direction exactly and the request below 4 pu,
       the input range of the multiplication by V_base / Vbus (Vbus <= 2 pu, so a
       pre-scaled request is always limited below). Bounded: fixed point at most 14
       halvings (2^31 / 2^17); float at most 18 (the PI output is below 2^20 pu). */
    while (m0 > SSTEP_ACC_IN_MAX)
    {
        x = sstep_acc_halve(x);
        y = sstep_acc_halve(y);
        m0 = sstep_acc_halve(m0);
    }

    /* relative to the bus voltage: 1.0 = Vbus */
    a = sstep_gain_mul_acc(inv_vbus, x);
    b = sstep_gain_mul_acc(inv_vbus, y);

#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1) && (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)
    m = (topology == SSTEP_TOPOLOGY_FULL_BRIDGE) ? span_full(a, b) : span_shared(a, b);
#elif (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1)
    (void)topology;
    m = span_full(a, b);
#else
    (void)topology;
    m = span_shared(a, b);
#endif

    out->limited = (m > SSTEP_ACC_ONE);
    if (out->limited)
    {
        a = scale_to_unit(a, m);
        b = scale_to_unit(b, m);
        /* applied voltage in pu of V_base: a * Vbus */
        out->v[0] = sstep_acc_scale(vbus, sstep_acc_to_real(a));
        out->v[1] = sstep_acc_scale(vbus, sstep_acc_to_real(b));
    }
    else
    {
        out->v[0] = va;
        out->v[1] = vb;
    }

#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1) && (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 1)
    if (topology == SSTEP_TOPOLOGY_FULL_BRIDGE)
    {
        duties_full(a, b, period, out);
    }
    else
    {
        duties_shared(a, b, period, out);
    }
#elif (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 1)
    duties_full(a, b, period, out);
#else
    duties_shared(a, b, period, out);
#endif
}
