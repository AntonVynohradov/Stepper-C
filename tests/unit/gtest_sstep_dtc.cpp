/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Dead-time compensation: term per phase, band, bus scaling, closed loop, observer.
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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "sstep_test_plant.hpp"

using sstep_test::Sim;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/* 1 us at 20 kHz: 0.02 of the bus per leg, 0.48 V at 24 V */
constexpr uint32_t kCompNs = 1000U;
constexpr uint32_t kCompBandMa = 250U;
constexpr double kVdead24 = 0.48;

void set_comp(Sim* sim, uint32_t band_ma = kCompBandMa)
{
    sim->cfg.stage.dead_time_comp_ns = kCompNs;
    sim->cfg.stage.dead_time_comp_band_ma = band_ma;
}

/* Voltage drive, locked rotor at angle 0 (phase A carries the reference, B none), plant
   without dead time: the compare values after t seconds */
void hold_compares(sstep_topology topo, bool comp, uint32_t i_ma, double vbus, double t,
                   uint16_t out[SSTEP_LEGS_MAX], uint16_t window = 0U)
{
    Sim sim(topo);
    sim.locked = true;
    sim.vbus = vbus;
    sim.cfg.stage.sample_window_ticks = window;
    sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
    sim.cfg.ctrl.i_run_ma = i_ma;
    sim.cfg.ctrl.i_hold_ma = i_ma;
    if (comp)
    {
        set_comp(&sim);
    }
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.run_for(t);
    for (unsigned k = 0; k < SSTEP_LEGS_MAX; k++)
    {
        out[k] = sim.hal.compare[k];
    }
}

int diff(const uint16_t c[SSTEP_LEGS_MAX], unsigned hi, unsigned lo)
{
    return static_cast<int>(c[hi]) - static_cast<int>(c[lo]);
}

double real_amps(sstep_real_t x)
{
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
    return static_cast<double>(x) / 32768.0 * sstep_test::kIFull;
#else
    return static_cast<double>(x) * sstep_test::kIFull;
#endif
}

}  // namespace

/* Off: the band alone changes nothing */
TEST(SstepDtc, OffChangesNothing)
{
    Sim a;
    Sim b;
    b.cfg.stage.dead_time_comp_band_ma = kCompBandMa;
    ASSERT_EQ(a.init(), SSTEP_OK);
    ASSERT_EQ(b.init(), SSTEP_OK);
    for (Sim* s : {&a, &b})
    {
        ASSERT_EQ(sstep_enable(&s->drv), SSTEP_OK);
        ASSERT_EQ(sstep_move_by(&s->drv, 64), SSTEP_OK);
    }
    for (int k = 0; k < 2000; k++)
    {
        a.step();
        b.step();
        ASSERT_EQ(std::memcmp(a.hal.compare, b.hal.compare, sizeof(a.hal.compare)), 0) << k;
    }
}

/* The term per phase in compare ticks: 2 Vd for the driven phase, Vd for the idle phase of
   the shared leg (W carries the current of A), 0 for the idle phase of the full bridge */
TEST(SstepDtc, TermShape)
{
    uint16_t off[SSTEP_LEGS_MAX];
    uint16_t on[SSTEP_LEGS_MAX];
    /* 2 t f P = 2 * 0.02 * 4250 = 170 ticks */
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, false, 2000U, 24.0, 0.03, off);
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, true, 2000U, 24.0, 0.03, on);
    EXPECT_NEAR(diff(on, 0, 2) - diff(off, 0, 2), 170, 2);
    EXPECT_NEAR(diff(on, 1, 2) - diff(off, 1, 2), 85, 2);
    hold_compares(SSTEP_TOPOLOGY_FULL_BRIDGE, false, 2000U, 24.0, 0.03, off);
    hold_compares(SSTEP_TOPOLOGY_FULL_BRIDGE, true, 2000U, 24.0, 0.03, on);
    EXPECT_NEAR(diff(on, 0, 1) - diff(off, 0, 1), 170, 2);
    EXPECT_NEAR(diff(on, 2, 3) - diff(off, 2, 3), 0, 1);
}

/* With a sampling window (W = P / 4, as low-side shunts need) the loss is a time of the full
   period: the same 170 / 85 ticks. Scaling it with (P - W) / P like the voltage limit would
   give 128 / 64 */
TEST(SstepDtc, TermShapeWithASamplingWindow)
{
    uint16_t off[SSTEP_LEGS_MAX];
    uint16_t on[SSTEP_LEGS_MAX];
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, false, 2000U, 24.0, 0.03, off, 1062U);
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, true, 2000U, 24.0, 0.03, on, 1062U);
    EXPECT_NEAR(diff(on, 0, 2) - diff(off, 0, 2), 170, 2);
    EXPECT_NEAR(diff(on, 1, 2) - diff(off, 1, 2), 85, 2);
}

/* Reference inside the band: 125 mA of 250 mA gives half the term, no step */
TEST(SstepDtc, TermInsideTheBand)
{
    uint16_t off[SSTEP_LEGS_MAX];
    uint16_t on[SSTEP_LEGS_MAX];
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, false, 125U, 24.0, 0.03, off);
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, true, 125U, 24.0, 0.03, on);
    /* A: Vd (0.5 + 0.5) = 85 ticks; B: Vd (0 + 0.5) = 42.5 ticks */
    EXPECT_NEAR(diff(on, 0, 2) - diff(off, 0, 2), 85, 2);
    EXPECT_NEAR(diff(on, 1, 2) - diff(off, 1, 2), 42.5, 2);
}

/* The loss follows the measured bus: at 12 V on the 24 V configuration the term is the same
   fraction of the bus (170 ticks, half the volts); from the nominal bus it would be 340 */
TEST(SstepDtc, ScalesWithTheMeasuredBus)
{
    uint16_t off[SSTEP_LEGS_MAX];
    uint16_t on[SSTEP_LEGS_MAX];
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, false, 1000U, 12.0, 0.03, off);
    hold_compares(SSTEP_TOPOLOGY_SHARED_LEG, true, 1000U, 12.0, 0.03, on);
    EXPECT_NEAR(diff(on, 0, 2) - diff(off, 0, 2), 170, 2);
}

/* Open-loop voltage drive against a plant with dead time: without the compensation the
   current stays short of the reference, with it it reaches it */
TEST(SstepDtc, VoltageDriveReachesTheCurrent)
{
    double ia[2];
    for (int comp = 0; comp < 2; comp++)
    {
        Sim sim;
        sim.locked = true;
        sim.v_dead = kVdead24;
        sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
        if (comp != 0)
        {
            set_comp(&sim);
        }
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.run_for(0.03);
        ia[comp] = sim.x.iA;
    }
    std::printf("[ measured ] voltage drive, 0.48 V dead time per leg: iA %.3f A without, "
                "%.3f A with compensation (reference 2 A)\n",
                ia[0], ia[1]);
    EXPECT_LT(ia[0], 1.5);
    EXPECT_NEAR(ia[1], 2.0, 0.05);
}

/* At the voltage limit the term goes through the normal scaling: no fault, compares in range */
TEST(SstepDtc, AtTheVoltageLimit)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_FULL_BRIDGE, SSTEP_TOPOLOGY_SHARED_LEG})
    {
        Sim sim(topo);
        sim.v_dead = kVdead24;
        set_comp(&sim);
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_set_motion_limits(&sim.drv, 25600, 320000, 0), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_run(&sim.drv, 25600), SSTEP_OK);
        uint16_t cmp_max = 0U;
        for (int k = 0; k < 3000; k++)
        {
            sim.step();
            for (unsigned j = 0; j < SSTEP_LEGS_MAX; j++)
            {
                cmp_max = std::max(cmp_max, sim.hal.compare[j]);
            }
        }
        sstep_status st{};
        ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
        EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
        EXPECT_LE(cmp_max, 4250U);
        EXPECT_GT(cmp_max, 4200U); /* the limit is used */
        EXPECT_LT(std::fabs(sim.load_angle_deg()), 90.0);
    }
}

/* Slow move against a plant with dead time: the compensation removes most of the current
   error at the zero crossings of the leg currents, which the PI controller has to catch up
   with. The band equals the 50 mA of the plant's dead-time model: a band wider than the real
   transition under-compensates there (250 mA against this plant: 16 -> 13 mA only). */
TEST(SstepDtc, ReducesZeroCrossingError)
{
    double err[2];
    for (int comp = 0; comp < 2; comp++)
    {
        Sim sim;
        sim.v_dead = kVdead24;
        sim.record = false;
        if (comp != 0)
        {
            set_comp(&sim, 50U);
        }
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_run(&sim.drv, 64), SSTEP_OK); /* one electrical period per second */
        sim.run_for(0.2);
        double e = 0.0;
        for (int k = 0; k < 40000; k++) /* 2 s */
        {
            const double ia = sim.x.iA;
            sim.step();
            const uint32_t used = sim.ctx().cur.angle - static_cast<uint32_t>(sim.ctx().cur.dangle);
            const double ref = real_amps(sim.ctx().cur.i_amp) *
                               std::cos(static_cast<double>(used) / 4294967296.0 * 2.0 * kPi);
            e = std::max(e, std::fabs(ia - ref));
        }
        err[comp] = e;
    }
    std::printf("[ measured ] 1 Hz electrical, 0.48 V dead time per leg: largest current "
                "error %.0f mA without, %.0f mA with compensation\n",
                err[0] * 1000.0, err[1] * 1000.0);
    EXPECT_LT(err[1] * 3.0, err[0]);
}

#if (SSTEP_CFG_STALL == 1)
/* The back-EMF observer sees the voltage at the coil: with the compensation the dead time no
   longer shows up as back-EMF along the current at rest */
TEST(SstepDtc, ObserverWithoutDeadTimeBias)
{
    double ed[2];
    for (int comp = 0; comp < 2; comp++)
    {
        Sim sim;
        sim.locked = true;
        sim.v_dead = kVdead24;
        sstep_test::set_stall(&sim.cfg);
        if (comp != 0)
        {
            set_comp(&sim);
        }
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.run_for(0.08); /* 2 A, before the hold current */
        sstep_status st{};
        ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
        ed[comp] = st.emf_d_mv;
    }
    std::printf("[ measured ] observer at rest, 0.48 V dead time per leg: back-EMF along the "
                "current %.0f mV without, %.0f mV with compensation\n",
                ed[0], ed[1]);
    EXPECT_GT(std::fabs(ed[0]), 300.0);
    EXPECT_LT(std::fabs(ed[1]), 50.0);
}
#endif /* SSTEP_CFG_STALL */
