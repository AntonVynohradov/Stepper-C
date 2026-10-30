/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Closed-loop tests: library + motor model through fake ADC/PWM (prototype T04..T09).
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

#include "sstep_test_plant.hpp"

using sstep_test::Record;
using sstep_test::Sim;

namespace
{

constexpr double kPi = 3.14159265358979323846;

double amplitude(const Record& r)
{
    return std::hypot(r.iA, r.iB);
}

} // namespace

/* T04: current step 0 -> 2 A at enable, locked rotor, both topologies */
TEST(SstepLoop, CurrentStep)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_FULL_BRIDGE, SSTEP_TOPOLOGY_SHARED_LEG})
    {
        Sim sim(topo);
        sim.locked = true;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.run_for(0.006);
        double peak = 0.0;
        double t_rise = -1.0;
        double t_settle = 0.0;
        for (const Record& r : sim.rec)
        {
            peak = std::max(peak, r.iA);
            if ((t_rise < 0.0) && (r.iA >= 1.8))
            {
                t_rise = r.t;
            }
            if (std::fabs(r.iA - 2.0) > 0.04)
            {
                t_settle = r.t + 5e-5;
            }
        }
        const double i_end = sim.rec.back().iA;
        std::printf("[ measured ] %s: overshoot %.2f %%, rise %.3f ms, settling %.3f ms, "
                    "final %.4f A, iB %.4f A\n",
                    topo == SSTEP_TOPOLOGY_FULL_BRIDGE ? "full" : "shared",
                    (peak - 2.0) / 2.0 * 100.0, t_rise * 1e3, t_settle * 1e3, i_end,
                    sim.rec.back().iB);
        /* prototype (Scilab/Python): 1.12 %, 0.35 ms, 0.40 ms */
        EXPECT_LT((peak - 2.0) / 2.0, 0.05);
        EXPECT_LT(t_settle, 1e-3);
        EXPECT_NEAR(i_end, 2.0, 0.004); /* ADC resolution 2 mA */
        EXPECT_NEAR(sim.rec.back().iB, 0.0, 0.004);
    }
}

/* T05: move 2 revolutions with the trapezoidal profile, free rotor */
TEST(SstepLoop, MoveTwoRevolutions)
{
    Sim sim;
    sim.record = false;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_move_to(&sim.drv, 6400), SSTEP_OK);
    double worst = 0.0;
    int done_at = -1;
    for (int k = 0; k < 12000; k++)
    {
        sim.step();
        worst = std::max(worst, std::fabs(sim.load_angle_deg()));
        if ((done_at < 0) && !sim.hal.events.empty())
        {
            done_at = k;
        }
    }
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    std::printf("[ measured ] move 6400 usteps: done at %.1f ms, max load angle %.2f deg, "
                "rotor %.2f usteps\n",
                done_at / 20.0, worst, sim.rotor_usteps());
    EXPECT_EQ(st.position, 6400);
    EXPECT_EQ(st.velocity, 0);
    EXPECT_EQ(st.state, SSTEP_STATE_HOLDING);
    EXPECT_LT(worst, 45.0);                       /* prototype: 15.2 deg */
    EXPECT_NEAR(sim.rotor_usteps(), 6400.0, 0.5); /* half a microstep */
    ASSERT_EQ(sim.hal.events.size(), 2U);
    EXPECT_EQ(sim.hal.events[0], SSTEP_EVENT_MOVE_DONE);
    EXPECT_EQ(sim.hal.events[1], SSTEP_EVENT_HOLD_CURRENT);
    EXPECT_NEAR(std::hypot(sim.x.iA, sim.x.iB), 1.0, 0.01); /* hold current */
}

/* T06: load step at 1 rev/s: synchronous below Kt*I, loss of synchronism above */
TEST(SstepLoop, LoadStep)
{
    for (double TL : {0.25, 0.5})
    {
        Sim sim;
        sim.record = false;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_run(&sim.drv, 3200), SSTEP_OK);
        sim.run_for(0.15);
        sim.TL = TL;
        double worst = 0.0;
        double sum_angle = 0.0;
        int n = 0;
        for (int k = 0; k < 3000; k++)
        {
            sim.step();
            worst = std::max(worst, std::fabs(sim.load_angle_deg()));
            if (k >= 2000)
            {
                const double p = sim.mot.Ns / 4.0;
                double d = (std::atan2(sim.x.iB, sim.x.iA) - p * sim.x.th) * 180.0 / kPi;
                d -= 360.0 * std::floor((d + 180.0) / 360.0);
                sum_angle += d;
                n++;
            }
        }
        if (TL < 0.4)
        {
            const double want = std::asin((TL + sim.mot.B * 2 * kPi) / (sim.mot.Kt * 2.0)) * 180 / kPi;
            std::printf("[ measured ] load %.2f N*m: current-rotor angle %.2f deg (asin %.2f), "
                        "max load angle %.1f deg\n",
                        TL, sum_angle / n, want, worst);
            EXPECT_LT(worst, 180.0);
            EXPECT_NEAR(sum_angle / n, want, 1.5);
        }
        else
        {
            std::printf("[ measured ] load %.2f N*m: max load angle %.0f deg (slip)\n", TL, worst);
            EXPECT_GT(worst, 180.0);
        }
    }
}

/* T07: reversal +4 -> -4 rev/s and stop */
TEST(SstepLoop, ReversalAndStop)
{
    Sim sim;
    sim.record = false;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_set_motion_limits(&sim.drv, 12800, 160000, 0), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_run(&sim.drv, 12800), SSTEP_OK);
    double worst = 0.0;
    for (int k = 0; k < 3000; k++)
    {
        sim.step();
        worst = std::max(worst, std::fabs(sim.load_angle_deg()));
    }
    ASSERT_EQ(sstep_run(&sim.drv, -12800), SSTEP_OK);
    for (int k = 0; k < 5000; k++)
    {
        sim.step();
        worst = std::max(worst, std::fabs(sim.load_angle_deg()));
    }
    const double rpm = sim.x.w * 60.0 / (2.0 * kPi);
    std::printf("[ measured ] reversal: max load angle %.2f deg, final %.1f rpm\n", worst, rpm);
    EXPECT_LT(worst, 90.0); /* prototype: 12.3 deg */
    EXPECT_NEAR(rpm, -240.0, 12.0);
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.velocity, -12800);
    EXPECT_EQ(st.state, SSTEP_STATE_MOVING);
    ASSERT_EQ(sstep_stop(&sim.drv), SSTEP_OK);
    sim.run_for(0.12);
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.velocity, 0);
    EXPECT_EQ(st.state, SSTEP_STATE_HOLDING);
    EXPECT_EQ(st.target, st.position);
    EXPECT_EQ(sstep_stop(&sim.drv), SSTEP_OK); /* already stopped */
}

/* T09: 8 rev/s: the shared leg reaches the voltage limit first */
TEST(SstepLoop, VoltageLimitByTopology)
{
    double amp[2] = {0.0, 0.0};
    int idx = 0;
    for (sstep_topology topo : {SSTEP_TOPOLOGY_FULL_BRIDGE, SSTEP_TOPOLOGY_SHARED_LEG})
    {
        Sim sim(topo);
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_set_motion_limits(&sim.drv, 25600, 320000, 0), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_run(&sim.drv, 25600), SSTEP_OK);
        sim.run_for(0.15);
        double sum = 0.0;
        int n = 0;
        for (const Record& r : sim.rec)
        {
            if (r.t >= 0.1)
            {
                sum += amplitude(r);
                n++;
            }
        }
        amp[idx] = sum / n;
        std::printf("[ measured ] 8 rev/s %s: current amplitude %.3f A (prototype %.3f A)\n",
                    topo == SSTEP_TOPOLOGY_FULL_BRIDGE ? "full" : "shared", amp[idx],
                    idx == 0 ? 1.862 : 1.557);
        EXPECT_LT(std::fabs(sim.load_angle_deg()), 90.0);
        idx++;
    }
    EXPECT_NEAR(amp[0], 1.862, 0.05);
    EXPECT_NEAR(amp[1], 1.557, 0.05);
    EXPECT_GT(amp[0] - amp[1], 0.2);
}

/* half the bus voltage: the duty cycle doubles, the current is the same */
TEST(SstepLoop, BusVoltageCompensation)
{
    Sim sim;
    sim.locked = true;
    sim.vbus = 12.0;
    sim.cfg.stage.vbus_min_mv = 0;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.run_for(0.01);
    EXPECT_NEAR(sim.x.iA, 2.0, 0.004);
}

/* voltage drive: V = R I, no current feedback (locked rotor: I settles at the reference) */
TEST(SstepLoop, VoltageDrive)
{
    Sim sim;
    sim.locked = true;
    sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.run_for(0.03);
    EXPECT_NEAR(sim.x.iA, 2.0, 0.01);
    EXPECT_NEAR(sim.x.iB, 0.0, 0.01);
}

/* microsteps 256 and 1, full bridge, move and come back */
TEST(SstepLoop, OtherMicrostepSettings)
{
    for (uint16_t m : {static_cast<uint16_t>(1), static_cast<uint16_t>(256)})
    {
        Sim sim(SSTEP_TOPOLOGY_FULL_BRIDGE);
        sim.record = false;
        sim.cfg.ctrl.microsteps = m;
        sim.cfg.ctrl.v_max = 1000U * m;
        sim.cfg.ctrl.accel = 10000U * m;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_move_to(&sim.drv, 50 * m), SSTEP_OK); /* 50 full steps */
        sim.run_for(0.3);
        EXPECT_NEAR(sim.rotor_usteps(), 50.0 * m, 0.05 * m + 0.5) << m;
        ASSERT_EQ(sstep_move_to(&sim.drv, 0), SSTEP_OK);
        sim.run_for(0.3);
        EXPECT_NEAR(sim.rotor_usteps(), 0.0, 0.05 * m + 0.5) << m;
    }
}
