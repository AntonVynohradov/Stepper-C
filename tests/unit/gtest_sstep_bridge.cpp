/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Unit tests of the averaged power stage mapping (both topologies, both backends).
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

#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>

#include "sstep_test_plant.hpp"

/* ==============================================================================
 * HELPERS
 * =============================================================================*/

namespace
{

constexpr uint16_t kPeriod = 4250U;

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
sstep_acc_t acc(double x)
{
    return static_cast<sstep_acc_t>(std::lround(x * 32768.0));
}
double accd(sstep_acc_t x)
{
    return x / 32768.0;
}
#else
sstep_acc_t acc(double x)
{
    return static_cast<sstep_acc_t>(x);
}
double accd(sstep_acc_t x)
{
    return x;
}
#endif

/* phase voltages [pu of Vbus] reconstructed from the compare values */
void from_compare(sstep_topology topo, const sstep_bridge_out& o, double* a, double* b)
{
    const double p = kPeriod;
    if (topo == SSTEP_TOPOLOGY_FULL_BRIDGE)
    {
        *a = (o.compare[0] - o.compare[1]) / p;
        *b = (o.compare[2] - o.compare[3]) / p;
    }
    else
    {
        *a = (o.compare[0] - o.compare[2]) / p;
        *b = (o.compare[1] - o.compare[2]) / p;
    }
}

struct Topo
{
    sstep_topology topo;
    const char* name;
    double circle; /* largest circular amplitude [pu of Vbus] */
};

const Topo kTopos[] = {{SSTEP_TOPOLOGY_FULL_BRIDGE, "full", 1.0},
                       {SSTEP_TOPOLOGY_SHARED_LEG, "shared", 1.0 / std::sqrt(2.0)}};

}  // namespace

/* ==============================================================================
 * TESTS
 * =============================================================================*/

TEST(SstepBridge, RandomRequests)
{
    sstep_gain inv{};
    ASSERT_EQ(sstep_gain_from_ratio(1, 1, &inv), SSTEP_OK); /* Vbus = V_base */
    for (const Topo& tp : kTopos)
    {
        uint32_t seed = 7U;
        double worst_dir = 0.0;
        double worst_exact = 0.0;
        bool duty_ok = true;
        for (int k = 0; k < 20000; k++)
        {
            seed = seed * 1664525U + 1013904223U;
            const double va = ((seed >> 8) % 20001) / 10000.0 * 3.0 - 3.0; /* -3 .. 3 pu */
            seed = seed * 1664525U + 1013904223U;
            const double vb = ((seed >> 8) % 20001) / 10000.0 * 3.0 - 3.0;
            sstep_bridge_out o{};
            sstep_bridge_map(tp.topo, acc(va), acc(vb), &inv, acc(1.0), kPeriod, &o);
            for (unsigned j = 0; j < (tp.topo == SSTEP_TOPOLOGY_FULL_BRIDGE ? 4U : 3U); j++)
            {
                duty_ok = duty_ok && (o.compare[j] <= kPeriod);
            }
            double a = 0.0;
            double b = 0.0;
            from_compare(tp.topo, o, &a, &b);
            /* direction kept: angle between request and output */
            /* direction error times output length = position error of the vector tip,
               bounded by the compare quantisation (two ticks) */
            const double cr = std::fabs(a * vb - b * va) / (std::hypot(va, vb) + 1e-12);
            worst_dir = std::fmax(worst_dir, cr);
            if (!o.limited)
            {
                worst_exact =
                    std::fmax(worst_exact, std::fmax(std::fabs(a - va), std::fabs(b - vb)));
            }
            /* reported applied voltage equals the compare values */
            EXPECT_NEAR(accd(o.v[0]), a, 3.0 / kPeriod);
            EXPECT_NEAR(accd(o.v[1]), b, 3.0 / kPeriod);
        }
        std::printf(
            "[ measured ] %s: direction error %.2e pu (vector tip), error when feasible %.2e pu "
            "(1 tick = %.2e)\n",
            tp.name, worst_dir, worst_exact, 1.0 / kPeriod);
        EXPECT_TRUE(duty_ok);
        EXPECT_LT(worst_dir, 2.0 / kPeriod);   /* compare quantisation */
        EXPECT_LT(worst_exact, 2.5 / kPeriod); /* one tick + rounding */
    }
}

TEST(SstepBridge, CircularAmplitude)
{
    sstep_gain inv{};
    ASSERT_EQ(sstep_gain_from_ratio(1, 1, &inv), SSTEP_OK);
    for (const Topo& tp : kTopos)
    {
        double amp = 1e9;
        for (int k = 0; k < 3600; k++)
        {
            const double ph = k * 2.0 * 3.14159265358979323846 / 3600.0;
            sstep_bridge_out o{};
            sstep_bridge_map(tp.topo, acc(3.0 * std::cos(ph)), acc(3.0 * std::sin(ph)), &inv,
                             acc(1.0), kPeriod, &o);
            double a = 0.0;
            double b = 0.0;
            from_compare(tp.topo, o, &a, &b);
            amp = std::fmin(amp, std::hypot(a, b));
            EXPECT_TRUE(o.limited);
        }
        EXPECT_NEAR(amp, tp.circle, 2e-3) << tp.name;
    }
}

TEST(SstepBridge, BusVoltageScaling)
{
    /* Vbus = V_base / 2: the same voltage needs twice the duty cycle */
    sstep_gain inv{};
    ASSERT_EQ(sstep_gain_from_ratio(2, 1, &inv), SSTEP_OK);
    for (const Topo& tp : kTopos)
    {
        sstep_bridge_out o{};
        sstep_bridge_map(tp.topo, acc(0.2), acc(-0.1), &inv, acc(0.5), kPeriod, &o);
        double a = 0.0;
        double b = 0.0;
        from_compare(tp.topo, o, &a, &b);
        EXPECT_FALSE(o.limited);
        EXPECT_NEAR(a, 0.4, 2.0 / kPeriod);
        EXPECT_NEAR(b, -0.2, 2.0 / kPeriod);
        /* limited: applied voltage = scaled duty times Vbus */
        sstep_bridge_map(tp.topo, acc(0.8), acc(0.0), &inv, acc(0.5), kPeriod, &o);
        EXPECT_TRUE(o.limited);
        EXPECT_NEAR(accd(o.v[0]), 0.5, 1e-3);
        EXPECT_NEAR(accd(o.v[1]), 0.0, 1e-3);
    }
}

TEST(SstepBridge, VeryLargeRequestKeepsDirection)
{
    /* far above 4 pu: power-of-two pre-scale before the multiplication */
    sstep_gain inv{};
    ASSERT_EQ(sstep_gain_from_ratio(1, 1, &inv), SSTEP_OK);
    for (const Topo& tp : kTopos)
    {
        sstep_bridge_out o{};
        sstep_bridge_map(tp.topo, acc(-300.0), acc(100.0), &inv, acc(1.0), kPeriod, &o);
        double a = 0.0;
        double b = 0.0;
        from_compare(tp.topo, o, &a, &b);
        EXPECT_TRUE(o.limited);
        EXPECT_NEAR(std::atan2(b, a), std::atan2(100.0, -300.0), 2e-3) << tp.name;
    }
}

TEST(SstepBridge, ZeroRequestCentred)
{
    sstep_gain inv{};
    ASSERT_EQ(sstep_gain_from_ratio(1, 1, &inv), SSTEP_OK);
    sstep_bridge_out o{};
    sstep_bridge_map(SSTEP_TOPOLOGY_SHARED_LEG, acc(0.0), acc(0.0), &inv, acc(1.0), kPeriod, &o);
    EXPECT_EQ(o.compare[0], kPeriod / 2U);
    EXPECT_EQ(o.compare[1], kPeriod / 2U);
    EXPECT_EQ(o.compare[2], kPeriod / 2U);
    EXPECT_EQ(o.compare[3], 0U);
}
