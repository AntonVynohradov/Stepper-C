/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Closed-loop tests: library + motor model through fake ADC/PWM (prototype T04..T09, stall T10..T12).
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
#include <functional>

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

}  // namespace

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
            const double want =
                std::asin((TL + sim.mot.B * 2 * kPi) / (sim.mot.Kt * 2.0)) * 180 / kPi;
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

/* ==============================================================================
 * STALL DETECTION ON THE MOTOR MODEL
 * =============================================================================*/

namespace
{

/* Nominal plant, or the worst case of Sim::make_worst_case() */
void stall_sim(Sim* sim, bool worst, bool detect)
{
    sim->record = false;
    sstep_test::set_reported_currents(&sim->cfg);
    if (detect)
    {
        sstep_test::set_stall(&sim->cfg);
    }
    if (worst)
    {
        sim->make_worst_case();
    }
}

const char* plant_name(bool worst)
{
    return worst ? "worst case" : "nominal";
}

}  // namespace

/* T10: the reported case. 0.12 N*m is held at 1.5 A (0.31 N*m) but not at the 0.5 A hold
   current (0.10 N*m): the rotor slips at the run -> hold change */
TEST(SstepLoop, StallAtHoldCurrent)
{
    for (bool worst : {false, true})
    {
        {
            Sim sim; /* without stall detection: the slip is not seen (1.0.0 behaviour) */
            stall_sim(&sim, worst, false);
            ASSERT_EQ(sim.init(), SSTEP_OK);
            ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
            sim.TL = 0.12;
            sim.run_for(0.8);
            EXPECT_GT(std::fabs(sim.load_angle_deg()), 360.0);
            EXPECT_EQ(sim.ctx().state, SSTEP_STATE_HOLDING);
            EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE);
        }
        Sim sim;
        stall_sim(&sim, worst, true);
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.TL = 0.12;
        double slip = -1.0;
        double trip = -1.0;
        for (int k = 0; (k < 16000) && (trip < 0.0); k++)
        {
            sim.step();
            const double t = (k + 1) / 20000.0;
            if ((slip < 0.0) && (std::fabs(sim.load_angle_deg()) > 90.0))
            {
                slip = t; /* beyond the pull-out angle */
            }
            if ((sim.ctx().faults & SSTEP_FAULT_STALL) != 0U)
            {
                trip = t;
            }
        }
        std::printf("[ measured ] hold-current slip (%s): load angle beyond 90 deg at %.1f ms, "
                    "SSTEP_FAULT_STALL at %.1f ms (%.1f ms later)\n",
                    plant_name(worst), slip * 1e3, trip * 1e3, (trip - slip) * 1e3);
        ASSERT_GT(slip, 0.5); /* held at the run current */
        ASSERT_GT(trip, slip);
        EXPECT_LT(trip - slip, 0.07);
        EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_STALL);
        EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
        EXPECT_FALSE(sim.hal.enabled);
        EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_STALL);
    }
}

/* T11: hard stop at 3 rev/s, both topologies */
TEST(SstepLoop, StallAtHardStop)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_FULL_BRIDGE, SSTEP_TOPOLOGY_SHARED_LEG})
    {
        Sim sim(topo);
        stall_sim(&sim, true, true);
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        ASSERT_EQ(sstep_run(&sim.drv, 9600), SSTEP_OK);
        sim.run_for(0.2);
        ASSERT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE);
        sim.kinematic = true;
        sim.x.w = 0.0;
        double trip = -1.0;
        for (int k = 0; (k < 4000) && (trip < 0.0); k++)
        {
            sim.step();
            if ((sim.ctx().faults & SSTEP_FAULT_STALL) != 0U)
            {
                trip = (k + 1) / 20000.0;
            }
        }
        std::printf("[ measured ] hard stop at 3 rev/s (%s, worst case): SSTEP_FAULT_STALL after "
                    "%.1f ms\n",
                    topo == SSTEP_TOPOLOGY_FULL_BRIDGE ? "full" : "shared", trip * 1e3);
        EXPECT_GT(trip, 0.05);
        EXPECT_LT(trip, 0.07);
    }
}

namespace
{

/* Drive at v [microsteps/s], 2 A, stall detection on, ramp 0.2 s */
void run_with_stall(Sim* sim, int32_t v)
{
    sim->record = false;
    sstep_test::set_stall(&sim->cfg);
    ASSERT_EQ(sim->init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim->drv), SSTEP_OK);
    ASSERT_EQ(sstep_set_motion_limits(&sim->drv, 40000, static_cast<uint32_t>(std::abs(v) * 5), 0),
              SSTEP_OK);
    ASSERT_EQ(sstep_run(&sim->drv, v), SSTEP_OK);
}

/* Raise the load to TL in steps of 0.02 N*m every 20 ms */
void load_stairs(Sim* sim, double TL)
{
    const int n = static_cast<int>(std::lround(std::fabs(TL) / 0.02));
    for (int k = 1; k <= n; k++)
    {
        sim->TL = 0.02 * k * (TL < 0.0 ? -1.0 : 1.0);
        sim->run_for(0.02);
    }
}

}  // namespace

/* T13: a heavy axis torn out of synchronism at low speed. Above the pull-out torque the rotor
   slips and the back-EMF vector turns in the commanded frame. A stall in 1.1.0 stayed
   unreported (hundreds of full steps) */
TEST(SstepLoop, StallOfAHeavyAxis)
{
    struct Case
    {
        double jx;
        int32_t v;
        bool worst;
    };
    // clang-format off: one case per line
    const Case cases[] = {
        {20.0, 3200, false}, {100.0, 3200, false}, {100.0, -3200, false},
        {20.0, 3200, true}, {100.0, 3200, true}, {20.0, 9600, false}, {100.0, 9600, false},
    };
    // clang-format on
    for (const Case& c : cases)
    {
        Sim sim;
        if (c.worst)
        {
            sim.make_worst_case();
        }
        sim.mot.J *= c.jx;
        run_with_stall(&sim, c.v);
        sim.run_for(0.3);
        ASSERT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE);
        sim.TL = (c.v > 0) ? 0.5 : -0.5;
        double slip = -1.0;
        double trip = -1.0;
        for (int k = 0; (k < 12000) && (trip < 0.0); k++)
        {
            sim.step();
            const double t = (k + 1) / 20000.0;
            if ((slip < 0.0) && (std::fabs(sim.load_angle_deg()) > 90.0))
            {
                slip = t;
            }
            if (sim.ctx().faults != SSTEP_FAULT_NONE)
            {
                trip = t;
            }
        }
        std::printf("[ measured ] heavy axis J x%.0f, %d usteps/s (%s), 0.5 N*m: fault 0x%02x "
                    "%.1f ms after the pull-out, %.0f full steps lost\n",
                    c.jx, c.v, plant_name(c.worst), static_cast<unsigned>(sim.ctx().faults),
                    (trip - slip) * 1e3, std::fabs(sim.load_angle_deg()) / 90.0);
        ASSERT_GT(slip, 0.0);
        ASSERT_GT(trip, slip) << c.jx << " " << c.v;
        EXPECT_NE(sim.ctx().faults & SSTEP_FAULT_STALL, 0U) << c.jx << " " << c.v;
        EXPECT_LT(trip - slip, 0.09) << c.jx << " " << c.v;
    }
}

/* T14: the bare rotor is torn away faster than stall_time_ms: the overcurrent trip stops the
   drive, with the stall flag because the detector had started */
TEST(SstepLoop, StallReportedWithTheOvercurrentOfAFastSlip)
{
    for (double jx : {1.0, 5.0})
    {
        Sim sim;
        sim.mot.J *= jx;
        run_with_stall(&sim, 3200);
        sim.run_for(0.3);
        ASSERT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE);
        sim.TL = 0.5;
        for (int k = 0; (k < 4000) && (sim.ctx().faults == SSTEP_FAULT_NONE); k++)
        {
            sim.step();
        }
        std::printf("[ measured ] fast slip J x%.0f: fault 0x%02x, %.0f full steps lost\n", jx,
                    static_cast<unsigned>(sim.ctx().faults),
                    std::fabs(sim.load_angle_deg()) / 90.0);
        EXPECT_NE(sim.ctx().faults & SSTEP_FAULT_OVERCURRENT, 0U) << jx;
        EXPECT_NE(sim.ctx().faults & SSTEP_FAULT_STALL, 0U) << jx;
    }
}

/* T15: heavy load held in synchronism on a hot winding with a saturated inductance: the error
   of L lowers the back-EMF along the motion, the resistance and the load angle show on d */
TEST(SstepLoop, StallNoFalseAlarmUnderLoad)
{
    struct Case
    {
        int32_t v;
        double TL;
        bool worst;
    };
    // clang-format off: one case per line
    const Case cases[] = {
        {3200, 0.30, false}, {-3200, -0.30, false}, {6400, 0.30, false}, {-6400, -0.30, false},
        {9600, 0.30, false}, {-9600, -0.30, false}, {3200, 0.25, true}, {-3200, -0.25, true},
    };
    // clang-format on
    for (const Case& c : cases)
    {
        Sim sim;
        if (c.worst)
        {
            sim.make_worst_case();
        }
        else
        {
            sim.mot.R *= 1.3;
            sim.mot.L *= 0.8;
        }
        run_with_stall(&sim, c.v);
        sim.run_for(0.3);
        load_stairs(&sim, c.TL);
        sim.run_for(0.2);
        std::printf("[ measured ] no stall under load %d usteps/s, %.2f N*m (%s): largest detector "
                    "level %u %%, load angle %.1f deg\n",
                    c.v, c.TL, c.worst ? "worst case" : "R +30 %, L -20 %",
                    100U * sim.stall_peak / sim.ctx().stall_ticks, sim.load_angle_deg());
        EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE) << c.v << " " << c.TL;
        EXPECT_LT(std::fabs(sim.load_angle_deg()), 90.0) << c.v;
    }
}

/* T16: the voltage limit on a lower bus voltage and with a larger inductance: the current lags
   its reference, the rotor stays synchronous */
TEST(SstepLoop, StallNoFalseAlarmAtTheVoltageLimit)
{
    struct Case
    {
        int32_t v;
        double vbus;
        double l;
    };
    for (const Case& c : {Case{25600, 18.0, 1.0}, Case{28800, 20.0, 1.0}, Case{35200, 24.0, 1.0},
                          Case{35200, 24.0, 1.2}})
    {
        Sim sim;
        sim.mot.L *= c.l;
        sim.vbus = c.vbus;
        sim.cfg.stage.vbus_nominal_mv = static_cast<uint32_t>(c.vbus * 1000.0);
        run_with_stall(&sim, c.v);
        sim.run_for(0.8);
        std::printf(
            "[ measured ] no stall at the voltage limit %d usteps/s, %.0f V, L x%.1f: largest "
            "detector level %u %%\n",
            c.v, c.vbus, c.l, 100U * sim.stall_peak / sim.ctx().stall_ticks);
        EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE) << c.v;
    }
}

/* T17: 11 rev/s at 20 V, no margin left: the load angle swings past 120 deg and is reported */
TEST(SstepLoop, StallOfALoadAngleSwingAtTheVoltageLimit)
{
    Sim sim;
    sim.vbus = 20.0;
    sim.cfg.stage.vbus_nominal_mv = 20000U;
    run_with_stall(&sim, 35200);
    double swing = -1.0;
    double trip = -1.0;
    double largest = 0.0;
    for (int k = 0; (k < 16000) && (trip < 0.0); k++)
    {
        sim.step();
        const double t = (k + 1) / 20000.0;
        largest = std::max(largest, std::fabs(sim.load_angle_deg()));
        if ((swing < 0.0) && (largest > 90.0))
        {
            swing = t;
        }
        if (sim.ctx().faults != SSTEP_FAULT_NONE)
        {
            trip = t;
        }
    }
    std::printf("[ measured ] load angle swing at 11 rev/s, 20 V: fault 0x%02x %.1f ms after the "
                "angle first passes 90 deg, largest angle %.1f deg\n",
                static_cast<unsigned>(sim.ctx().faults), (trip - swing) * 1e3, largest);
    ASSERT_GT(swing, 0.0);
    ASSERT_GT(trip, swing);
    EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_STALL);
    EXPECT_LT(largest, 180.0); /* no step lost yet */
}

/* T12: no stall fault in normal operation, nominal and worst-case plant */
TEST(SstepLoop, StallNoFalseAlarm)
{
    struct Scenario
    {
        const char* name;
        std::function<void(Sim&)> run;
    };
    auto run_v = [](int32_t v) {
        return [v](Sim& s) {
            ASSERT_EQ(sstep_set_motion_limits(&s.drv, 40000, 160000, 0), SSTEP_OK);
            ASSERT_EQ(sstep_run(&s.drv, v), SSTEP_OK);
            s.run_for(0.8);
        };
    };
    // clang-format off: one scenario per line
    const Scenario scenarios[] = {
        {"enable, run -> hold", [](Sim& s) { s.run_for(0.8); }},
        {"0.07 N*m through run -> hold", [](Sim& s) { s.TL = 0.07; s.run_for(0.8); }},
        {"move 6400 usteps, hold", [](Sim& s) { ASSERT_EQ(sstep_move_to(&s.drv, 6400), SSTEP_OK); s.run_for(1.2); }},
        {"reversal +-4 rev/s, stop", [](Sim& s) { ASSERT_EQ(sstep_run(&s.drv, 12800), SSTEP_OK); s.run_for(0.2); ASSERT_EQ(sstep_run(&s.drv, -12800), SSTEP_OK); s.run_for(0.3); ASSERT_EQ(sstep_stop(&s.drv), SSTEP_OK); s.run_for(0.3); }},
        {"0.125 rev/s", run_v(400)},
        {"0.25 rev/s", run_v(800)},
        {"0.47 rev/s", run_v(1500)},
        {"1 rev/s", run_v(3200)},
        {"2 rev/s", run_v(6400)},
        {"8 rev/s", run_v(25600)},
        {"9.5 rev/s", run_v(30400)},
        {"11 rev/s", run_v(35200)},
    };
    // clang-format on
    for (bool worst : {false, true})
    {
        for (const Scenario& sc : scenarios)
        {
            Sim sim;
            stall_sim(&sim, worst, true);
            ASSERT_EQ(sim.init(), SSTEP_OK);
            ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
            sc.run(sim);
            std::printf("[ measured ] no stall (%s) %s: largest detector level %u %%\n",
                        plant_name(worst), sc.name, 100U * sim.stall_peak / sim.ctx().stall_ticks);
            EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_NONE) << plant_name(worst) << " " << sc.name;
            EXPECT_LT(std::fabs(sim.load_angle_deg()), 90.0) << sc.name;
        }
    }
}
