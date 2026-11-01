/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Unit tests of the stall detection with synthetic signals: imposed rotor motion and injected observer states.
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
#include <functional>

#include "sstep_test_plant.hpp"

using sstep_test::Hal;
using sstep_test::make_config;
using sstep_test::Sim;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/* Ke w [mV] of a velocity in microsteps/s (17HS19, 200 steps, 16 microsteps) */
double emf_mv(double usteps_per_s)
{
    return 0.2086 * usteps_per_s * 2.0 * kPi / 3200.0 * 1000.0;
}

/* Rotor speed [rad/s] of a velocity in microsteps/s */
double rad_s(double usteps_per_s)
{
    return usteps_per_s * 2.0 * kPi / 3200.0;
}

/* Reference driver with stall detection, outputs enabled */
void start(Sim* sim)
{
    sim->record = false;
    sstep_test::set_stall(&sim->cfg);
    ASSERT_EQ(sim->init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim->drv), SSTEP_OK);
}

/* Run until the stall fault, at most max_s; returns the time [s] or -1 */
double run_until_stall(Sim* sim, double max_s)
{
    const auto n = static_cast<int>(std::lround(max_s * 20000.0));
    double t = -1.0;
    for (int k = 0; (k < n) && (t < 0.0); k++)
    {
        sim->step();
        if ((sim->ctx().faults & SSTEP_FAULT_STALL) != 0U)
        {
            t = (k + 1) / 20000.0;
        }
    }
    return t;
}

sstep_status status_of(Sim* sim)
{
    sstep_status st{};
    EXPECT_EQ(sstep_get_status(&sim->drv, &st), SSTEP_OK);
    return st;
}

}  // namespace

/* ==============================================================================
 * CONFIGURATION
 * =============================================================================*/

TEST(SstepStall, DisabledByDefault)
{
    Sim sim;
    sim.record = false;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sim.ctx().stall_ticks, 0U);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.kinematic = true; /* the rotor turns, nobody looks */
    sim.x.w = rad_s(6400.0);
    sim.run_for(0.2);
    const sstep_status st = status_of(&sim);
    EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
    EXPECT_EQ(st.emf_d_mv, 0);
    EXPECT_EQ(st.emf_q_mv, 0);
    EXPECT_EQ(st.rotor_velocity, 0);
    EXPECT_EQ(st.load_angle_cdeg, 0);
    EXPECT_EQ(st.stall_level_pct, 0U);
}

TEST(SstepStall, ConfigChecks)
{
    using Mod = std::function<void(sstep_config&)>;
    struct Case
    {
        const char* name;
        Mod mod;
        int expect;
    };
    // clang-format off: one case per line
    const Case cases[] = {
        {"reference", [](sstep_config&) {}, SSTEP_OK},
        {"off ignores the other fields", [](sstep_config& c) { c.ctrl.stall_time_ms = 0; c.ctrl.stall_filter_us = 0; c.ctrl.stall_speed = 0; c.ctrl.stall_sync_pct = 500; }, SSTEP_OK},
        {"time huge", [](sstep_config& c) { c.ctrl.stall_time_ms = 20000000; }, SSTEP_ERR_PARAM},
        {"filter 0", [](sstep_config& c) { c.ctrl.stall_filter_us = 0; }, SSTEP_ERR_PARAM},
        {"filter huge", [](sstep_config& c) { c.ctrl.stall_filter_us = 20000000; }, SSTEP_ERR_PARAM},
        {"speed 0", [](sstep_config& c) { c.ctrl.stall_speed = 0; }, SSTEP_ERR_PARAM},
        {"speed huge", [](sstep_config& c) { c.ctrl.stall_speed = 20000000; }, SSTEP_ERR_PARAM},
        {"sync 0", [](sstep_config& c) { c.ctrl.stall_sync_pct = 0; }, SSTEP_ERR_PARAM},
        {"sync 101", [](sstep_config& c) { c.ctrl.stall_sync_pct = 101; }, SSTEP_ERR_PARAM},
        {"voltage drive", [](sstep_config& c) { c.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE; }, SSTEP_ERR_CONFIG},
        {"no Ke", [](sstep_config& c) { c.motor.ke_uv_s_per_rad = 0; }, SSTEP_ERR_CONFIG},
        {"filter below Ts", [](sstep_config& c) { c.ctrl.stall_filter_us = 49; }, SSTEP_ERR_CONFIG},
        {"filter one Ts", [](sstep_config& c) { c.ctrl.stall_filter_us = 50; }, SSTEP_OK},
        {"time below a slow period", [](sstep_config& c) { c.ctrl.slow_loop_hz = 100; c.ctrl.stall_time_ms = 9; }, SSTEP_ERR_CONFIG},
        {"time above 2^32 periods", [](sstep_config& c) { c.ctrl.fast_loop_hz = 9000000; c.ctrl.slow_loop_hz = 9000000; c.ctrl.accel = 2000000000; c.ctrl.stall_time_ms = 9000000; }, SSTEP_ERR_CONFIG},
        {"threshold below 1 mV", [](sstep_config& c) { c.ctrl.stall_speed = 1; }, SSTEP_ERR_CONFIG}, /* 0.41 mV */
        {"threshold 1 mV", [](sstep_config& c) { c.ctrl.stall_speed = 2; }, SSTEP_OK},             /* 0.82 mV */
        {"threshold above the bus", [](sstep_config& c) { c.ctrl.stall_speed = 60000; }, SSTEP_ERR_CONFIG}, /* 24.6 V */
        {"L / Ts overflow", [](sstep_config& c) { c.motor.l_uh = 9000000; c.ctrl.fast_loop_hz = 9000000; c.ctrl.current_bw_hz = 1; c.stage.i_full_scale_ma = 9000000; c.stage.i_trip_ma = 9000000; }, SSTEP_ERR_CONFIG},
        {"L / Ts above 32767", [](sstep_config& c) { c.motor.l_uh = 9000000; c.ctrl.current_bw_hz = 1; c.stage.vbus_nominal_mv = 20000; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"L / Ts below R / 2", [](sstep_config& c) { c.motor.l_uh = 10; }, SSTEP_OK},
    };
    // clang-format on
    for (const Case& k : cases)
    {
        Hal hal;
        sstep drv{};
        sstep_config cfg = make_config(&hal);
        sstep_test::set_stall(&cfg);
        k.mod(cfg);
        EXPECT_EQ(sstep_init(&drv, &cfg), k.expect) << k.name;
        cfg.ctrl.stall_time_ms = 0U; /* each case is about the stall fields only */
        EXPECT_EQ(sstep_init(&drv, &cfg), SSTEP_OK) << k.name;
    }
}

namespace
{

double gain_value(const sstep_gain& g)
{
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
    return std::ldexp(static_cast<double>(g.mant) / 32768.0, g.exp);
#else
    return static_cast<double>(g.value);
#endif
}

}  // namespace

TEST(SstepStall, ObserverGains)
{
    /* a, b = (L fs +/- R / 2) I_base / V_base, Ts / tau */
    struct Case
    {
        uint32_t l_uh;
        double a, b;
    };
    for (const Case& c : {Case{3000U, 10.1333, 9.8667}, Case{10U, 0.1667, -0.1}})
    {
        Sim sim;
        sstep_test::set_stall(&sim.cfg);
        sim.cfg.motor.l_uh = c.l_uh;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        EXPECT_NEAR(gain_value(sim.ctx().emf_a), c.a, 1e-3 * std::fabs(c.a)) << c.l_uh;
        EXPECT_NEAR(gain_value(sim.ctx().emf_b), c.b, 1e-3 * std::fabs(c.b)) << c.l_uh;
        EXPECT_NEAR(gain_value(sim.ctx().emf_alpha), 0.05, 1e-5);
        EXPECT_EQ(sim.ctx().stall_ticks, 50U);
        EXPECT_EQ(sim.ctx().stall_emf_mv, 655); /* 0.2086 * 2 pi * 0.5 rev/s */
    }
}

/* ==============================================================================
 * OBSERVER: rotor imposed at the commanded speed
 * =============================================================================*/

TEST(SstepStall, ObserverMeasuresTheBackEmf)
{
    for (int32_t v : {9600, -9600})
    {
        Sim sim;
        start(&sim);
        ASSERT_EQ(sstep_run(&sim.drv, v), SSTEP_OK);
        sim.run_for(0.3); /* free rotor, synchronous */
        sim.kinematic = true;
        sim.x.w = rad_s(v); /* exactly the commanded speed from now on */
        sim.run_for(0.05);
        const sstep_status st = status_of(&sim);
        /* the estimate refers the rotor at the middle of the previous period to the
           commanded angle of this period: 1.5 periods later than load_angle_deg() */
        const double lead = 1.5 * 50.0 * rad_s(v) / 20000.0 * 180.0 / kPi;
        const double delta = sim.load_angle_deg() + lead;
        const double mag = std::hypot(st.emf_d_mv, st.emf_q_mv);
        std::printf("[ measured ] observer at %d usteps/s: |e| %.0f mV (Ke w %.0f mV), "
                    "rotor %d usteps/s, load angle %.2f deg (model %.2f deg)\n",
                    v, mag, emf_mv(std::fabs(v)), st.rotor_velocity, st.load_angle_cdeg / 100.0,
                    delta);
        EXPECT_NEAR(mag, emf_mv(std::fabs(v)), 0.01 * emf_mv(std::fabs(v)));
        EXPECT_NEAR(st.rotor_velocity, v, 0.01 * std::fabs(v));
        EXPECT_NEAR(st.load_angle_cdeg / 100.0, delta, 0.5);
        EXPECT_EQ(st.stall_level_pct, 0U);
        EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
    }
}

/* ==============================================================================
 * STALL AT REST: the command holds, the rotor turns
 * =============================================================================*/

TEST(SstepStall, RotorTurningAtRestTrips)
{
    Sim sim;
    start(&sim);
    sim.run_for(0.1);
    sim.kinematic = true;
    sim.x.w = rad_s(3200.0); /* 1 rev/s, twice the threshold */
    const double t = run_until_stall(&sim, 0.5);
    std::printf("[ measured ] rotor at 3200 usteps/s, command at rest: stall after %.1f ms\n",
                t * 1e3);
    ASSERT_GT(t, 0.0);
    EXPECT_GT(t, 0.050); /* debounced: never before stall_time_ms */
    EXPECT_LT(t, 0.060); /* filter and slow-loop sampling add a few ms */

    /* the usual fault behaviour */
    const sstep_status st = status_of(&sim);
    EXPECT_EQ(st.state, SSTEP_STATE_FAULT);
    EXPECT_EQ(st.faults, SSTEP_FAULT_STALL);
    EXPECT_EQ(st.stall_level_pct, 100U);
    EXPECT_FALSE(sim.hal.enabled);
    EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_STALL);
    EXPECT_EQ(sim.hal.fault_calls, 1);
    EXPECT_EQ(sstep_move_to(&sim.drv, 100), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_STATE);
    ASSERT_EQ(sstep_clear_faults(&sim.drv), SSTEP_OK);
    EXPECT_EQ(status_of(&sim).state, SSTEP_STATE_DISABLED);

    /* enabled again: the detector starts from zero */
    sim.x.w = 0.0;
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    EXPECT_EQ(sim.ctx().stall_count, 0U);
    EXPECT_EQ(sim.ctx().emf[0], SSTEP_ACC_ZERO);
    EXPECT_EQ(sim.ctx().emf[1], SSTEP_ACC_ZERO);
    sim.run_for(0.2);
    EXPECT_EQ(status_of(&sim).state, SSTEP_STATE_HOLDING);
}

TEST(SstepStall, RotorSlowerThanTheThresholdIsIgnored)
{
    Sim sim;
    start(&sim);
    sim.kinematic = true;
    sim.x.w = rad_s(800.0); /* half the threshold */
    EXPECT_LT(run_until_stall(&sim, 0.5), 0.0);
    EXPECT_EQ(status_of(&sim).state, SSTEP_STATE_HOLDING);
}

TEST(SstepStall, RotorFasterThanASlowCommandTrips)
{
    /* below stall_speed the command is "slow": the rotor must not exceed it by more */
    Sim sim;
    start(&sim);
    ASSERT_EQ(sstep_run(&sim.drv, -400), SSTEP_OK);
    sim.run_for(0.1);
    EXPECT_EQ(status_of(&sim).faults, SSTEP_FAULT_NONE);
    sim.kinematic = true;
    sim.x.w = rad_s(-3200.0);
    EXPECT_GT(run_until_stall(&sim, 0.2), 0.0);
}

/* ==============================================================================
 * STALL IN MOTION: the rotor stops, the command goes on
 * =============================================================================*/

TEST(SstepStall, RotorBlockedInMotionTrips)
{
    for (int32_t v : {9600, -9600})
    {
        Sim sim;
        start(&sim);
        ASSERT_EQ(sstep_run(&sim.drv, v), SSTEP_OK);
        sim.run_for(0.2);
        EXPECT_EQ(status_of(&sim).faults, SSTEP_FAULT_NONE);
        sim.kinematic = true;
        sim.x.w = 0.0; /* hard stop */
        const double t = run_until_stall(&sim, 0.3);
        std::printf("[ measured ] rotor blocked at %d usteps/s: stall after %.1f ms\n", v, t * 1e3);
        ASSERT_GT(t, 0.050);
        EXPECT_LT(t, 0.060);
        EXPECT_EQ(status_of(&sim).state, SSTEP_STATE_FAULT);
    }
}

/* ==============================================================================
 * DECISION AND STATUS: injected observer states
 * =============================================================================*/

namespace
{

/* Filter state of a back-EMF in mV (24 V base) */
sstep_integ_t emf_state(double mv)
{
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
    return static_cast<sstep_integ_t>(std::lround(mv / 24000.0 * 134217728.0)); /* Q27 */
#else
    return static_cast<sstep_integ_t>(mv / 24000.0);
#endif
}

}  // namespace

TEST(SstepStall, IntermittentConditionDoesNotTrip)
{
    /* up/down counter: in motion, every other period without back-EMF never trips */
    Sim sim;
    start(&sim);
    ASSERT_EQ(sstep_run(&sim.drv, 9600), SSTEP_OK);
    for (int k = 0; k < 400; k++)
    {
        const bool good = (k < 200) || ((k % 2) == 0);
        sim.ctx().emf[1] = emf_state(good ? emf_mv(sim.ctx().vel_cmd) : 0.0);
        ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    }
    EXPECT_EQ(sim.ctx().vel_cmd, 9600);
    EXPECT_LE(sim.ctx().stall_count, 1U);
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_MOVING);
    for (int k = 0; k < 50; k++) /* then 50 periods in a row trip */
    {
        sim.ctx().emf[1] = emf_state(0.0);
        ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    }
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
    EXPECT_EQ(sim.ctx().faults, SSTEP_FAULT_STALL);
}

TEST(SstepStall, OffsetAtRestIsLearned)
{
    /* a constant observer error (resistance, dead time) at rest is not rotor motion */
    Sim sim;
    start(&sim);
    for (int k = 0; k < 1000; k++)
    {
        sim.ctx().emf[0] = emf_state(1200.0); /* 1.8 x the 655 mV threshold */
        sim.ctx().emf[1] = emf_state(-300.0);
        ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    }
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_HOLDING);
    EXPECT_EQ(sim.ctx().stall_count, 0U);
    EXPECT_NEAR(static_cast<double>(sim.ctx().emf_off[0]) / 65536.0, 1200.0, 2.0);
    EXPECT_NEAR(static_cast<double>(sim.ctx().emf_off[1]) / 65536.0, -300.0, 2.0);
}

TEST(SstepStall, StatusFromTheBackEmfVector)
{
    struct Case
    {
        int32_t vel; /* commanded velocity of the period [microsteps/s] */
        double ed, eq;
        double angle; /* expected load angle [deg] */
        int32_t sign; /* expected sign of the rotor velocity */
    };
    const double e = emf_mv(6400.0);
    const double s30 = 0.5;
    const double c30 = std::sqrt(3.0) / 2.0;
    // clang-format off: one case per line
    const Case cases[] = {
        {6400, e * s30, e * c30, 30.0, 1},     /* forward, rotor 30 deg behind: motoring */
        {6400, -e * s30, e * c30, -30.0, 1},   /* forward, rotor ahead: braking */
        {-6400, e * s30, -e * c30, -30.0, -1}, /* reverse, rotor behind: negative torque */
        {-6400, -e * s30, -e * c30, 30.0, -1}, /* reverse, rotor ahead */
        {6400, e, 0.0, 90.0, 1},                /* pull-out angle */
        {6400, e * 0.7071, -e * 0.7071, 135.0, -1}, /* rotor turning backward */
        {0, e * s30, -e * c30, -30.0, -1},     /* at rest: direction of the back-EMF */
        {0, 0.0, 0.0, 0.0, 0},                 /* nothing */
    };
    // clang-format on
    for (const Case& c : cases)
    {
        Sim sim;
        start(&sim);
        sim.ctx().vel_cmd = c.vel;
        sim.ctx().emf[0] = emf_state(c.ed);
        sim.ctx().emf[1] = emf_state(c.eq);
        const sstep_status st = status_of(&sim);
        EXPECT_NEAR(st.load_angle_cdeg / 100.0, c.angle, 0.05) << c.vel << " " << c.angle;
        EXPECT_NEAR(st.rotor_velocity, c.sign * 6400, 6) << c.vel << " " << c.angle;
        EXPECT_NEAR(st.emf_d_mv, c.ed, 1.0);
        EXPECT_NEAR(st.emf_q_mv, c.eq, 1.0);
    }
}

TEST(SstepStall, RotorVelocitySaturates)
{
    /* Ke of 1 uV*s/rad, 256 microsteps: half the bus voltage means 9.8e10 microsteps/s */
    Sim sim;
    sim.record = false;
    sstep_test::set_stall(&sim.cfg);
    sim.cfg.motor.ke_uv_s_per_rad = 1U;
    sim.cfg.ctrl.microsteps = 256U;
    sim.cfg.ctrl.stall_speed = 10000000U; /* 1.2 mV */
    ASSERT_EQ(sim.init(), SSTEP_OK);
    for (double mv : {12000.0, -12000.0})
    {
        sim.ctx().emf[1] = emf_state(mv);
        EXPECT_EQ(status_of(&sim).rotor_velocity, mv > 0.0 ? INT32_MAX : -INT32_MAX);
    }
}
