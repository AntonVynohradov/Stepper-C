/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Unit tests of the public API: configuration checks, states, commands, faults, status.
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
#include <cstring>
#include <functional>
#include <string>

#include "sstep_test_plant.hpp"

using sstep_test::Hal;
using sstep_test::make_config;
using sstep_test::Sim;

/* ==============================================================================
 * CONFIGURATION
 * =============================================================================*/

TEST(SstepApi, InitAcceptsTheReferenceConfiguration)
{
    Hal hal;
    sstep drv{};
    const sstep_config cfg = make_config(&hal);
    ASSERT_EQ(sstep_init(&drv, &cfg), SSTEP_OK);
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_DISABLED);
    EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
    EXPECT_FALSE(hal.enabled);
}

TEST(SstepApi, InitRejectsNull)
{
    Hal hal;
    sstep drv{};
    const sstep_config cfg = make_config(&hal);
    EXPECT_EQ(sstep_init(nullptr, &cfg), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_init(&drv, nullptr), SSTEP_ERR_PARAM);
}

TEST(SstepApi, InitRejectsInvalidFields)
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
        {"pwm_write NULL", [](sstep_config& c) { c.hal.pwm_write = nullptr; }, SSTEP_ERR_PARAM},
        {"output_enable NULL", [](sstep_config& c) { c.hal.output_enable = nullptr; }, SSTEP_ERR_PARAM},
        {"R 0", [](sstep_config& c) { c.motor.r_mohm = 0; }, SSTEP_ERR_PARAM},
        {"R huge", [](sstep_config& c) { c.motor.r_mohm = 20000000; }, SSTEP_ERR_PARAM},
        {"L 0", [](sstep_config& c) { c.motor.l_uh = 0; }, SSTEP_ERR_PARAM},
        {"L huge", [](sstep_config& c) { c.motor.l_uh = 20000000; }, SSTEP_ERR_PARAM},
        {"I rated 0", [](sstep_config& c) { c.motor.i_rated_ma = 0; }, SSTEP_ERR_PARAM},
        {"I rated huge", [](sstep_config& c) { c.motor.i_rated_ma = 20000000; }, SSTEP_ERR_PARAM},
        {"ke huge", [](sstep_config& c) { c.motor.ke_uv_s_per_rad = 20000000; }, SSTEP_ERR_PARAM},
        {"steps 0", [](sstep_config& c) { c.motor.full_steps_per_rev = 0; }, SSTEP_ERR_PARAM},
        {"topology", [](sstep_config& c) { c.stage.topology = static_cast<sstep_topology>(7); }, SSTEP_ERR_PARAM},
        {"Vbus 0", [](sstep_config& c) { c.stage.vbus_nominal_mv = 0; }, SSTEP_ERR_PARAM},
        {"Vbus huge", [](sstep_config& c) { c.stage.vbus_nominal_mv = 20000000; }, SSTEP_ERR_PARAM},
        {"Vmin huge", [](sstep_config& c) { c.stage.vbus_min_mv = 20000000; }, SSTEP_ERR_PARAM},
        {"Vmax huge", [](sstep_config& c) { c.stage.vbus_max_mv = 20000000; }, SSTEP_ERR_PARAM},
        {"I full 0", [](sstep_config& c) { c.stage.i_full_scale_ma = 0; }, SSTEP_ERR_PARAM},
        {"I full huge", [](sstep_config& c) { c.stage.i_full_scale_ma = 20000000; }, SSTEP_ERR_PARAM},
        {"I trip 0", [](sstep_config& c) { c.stage.i_trip_ma = 0; }, SSTEP_ERR_PARAM},
        {"period 1", [](sstep_config& c) { c.stage.pwm_period_ticks = 1; }, SSTEP_ERR_PARAM},
        {"span 0", [](sstep_config& c) { c.stage.adc_i_span = 0; }, SSTEP_ERR_PARAM},
        {"span huge", [](sstep_config& c) { c.stage.adc_i_span = 70000; }, SSTEP_ERR_PARAM},
        {"span -huge", [](sstep_config& c) { c.stage.adc_i_span = -70000; }, SSTEP_ERR_PARAM},
        {"adc full 0", [](sstep_config& c) { c.stage.adc_full = 0; }, SSTEP_ERR_PARAM},
        {"adc zero", [](sstep_config& c) { c.stage.adc_i_zero = 5000; }, SSTEP_ERR_PARAM},
        {"adc vbus 0", [](sstep_config& c) { c.stage.adc_vbus_mv_full = 0; }, SSTEP_ERR_PARAM},
        {"adc vbus huge", [](sstep_config& c) { c.stage.adc_vbus_mv_full = 20000000; }, SSTEP_ERR_PARAM},
        {"drive mode", [](sstep_config& c) { c.ctrl.drive_mode = static_cast<sstep_drive_mode>(5); }, SSTEP_ERR_PARAM},
        {"fast 0", [](sstep_config& c) { c.ctrl.fast_loop_hz = 0; }, SSTEP_ERR_PARAM},
        {"slow 0", [](sstep_config& c) { c.ctrl.slow_loop_hz = 0; }, SSTEP_ERR_PARAM},
        {"fast huge", [](sstep_config& c) { c.ctrl.fast_loop_hz = 20000000; }, SSTEP_ERR_PARAM},
        {"microsteps 0", [](sstep_config& c) { c.ctrl.microsteps = 0; }, SSTEP_ERR_PARAM},
        {"microsteps 3", [](sstep_config& c) { c.ctrl.microsteps = 3; }, SSTEP_ERR_PARAM},
        {"microsteps 512", [](sstep_config& c) { c.ctrl.microsteps = 512; }, SSTEP_ERR_PARAM},
        {"bw 0", [](sstep_config& c) { c.ctrl.current_bw_hz = 0; }, SSTEP_ERR_PARAM},
        {"i_run 0", [](sstep_config& c) { c.ctrl.i_run_ma = 0; }, SSTEP_ERR_PARAM},
        {"v_max 0", [](sstep_config& c) { c.ctrl.v_max = 0; }, SSTEP_ERR_PARAM},
        {"accel 0", [](sstep_config& c) { c.ctrl.accel = 0; }, SSTEP_ERR_PARAM},
        {"trip > full", [](sstep_config& c) { c.stage.i_trip_ma = 5000; }, SSTEP_ERR_CONFIG},
        {"fast % slow", [](sstep_config& c) { c.ctrl.slow_loop_hz = 999; }, SSTEP_ERR_CONFIG},
        {"ratio > 65535", [](sstep_config& c) { c.ctrl.fast_loop_hz = 9000000; c.ctrl.slow_loop_hz = 100; }, SSTEP_ERR_CONFIG},
        {"bw > fs/4", [](sstep_config& c) { c.ctrl.current_bw_hz = 6000; }, SSTEP_ERR_CONFIG},
        {"Vmin >= Vnom", [](sstep_config& c) { c.stage.vbus_min_mv = 24000; }, SSTEP_ERR_CONFIG},
        {"Vmax <= Vnom", [](sstep_config& c) { c.stage.vbus_max_mv = 24000; }, SSTEP_ERR_CONFIG},
        {"i_run > trip", [](sstep_config& c) { c.motor.i_rated_ma = 4000; c.ctrl.i_run_ma = 3600; }, SSTEP_ERR_CONFIG},
        {"i_run > rated", [](sstep_config& c) { c.ctrl.i_run_ma = 2100; }, SSTEP_ERR_CONFIG},
        {"hold > run", [](sstep_config& c) { c.ctrl.i_hold_ma = 2100; }, SSTEP_ERR_CONFIG},
        {"trip below 1 count", [](sstep_config& c) { c.stage.adc_i_span = 1; c.stage.i_trip_ma = 100; c.ctrl.i_run_ma = 100; c.ctrl.i_hold_ma = 0; }, SSTEP_ERR_CONFIG},
        {"v_max too small", [](sstep_config& c) { c.ctrl.v_max = 1; c.ctrl.slow_loop_hz = 200000; c.ctrl.fast_loop_hz = 200000; }, SSTEP_ERR_PARAM},
        {"accel too small", [](sstep_config& c) { c.ctrl.accel = 5; }, SSTEP_ERR_PARAM}, /* < 0.5 Q16 */
        {"Kp overflow", [](sstep_config& c) { c.motor.l_uh = 9000000; c.stage.i_full_scale_ma = 9000000; c.stage.i_trip_ma = 3500; }, SSTEP_ERR_CONFIG},
        {"Ki overflow", [](sstep_config& c) { c.ctrl.fast_loop_hz = 9000000; c.ctrl.slow_loop_hz = 1000; c.stage.vbus_nominal_mv = 9000000; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"Kp above 32767", [](sstep_config& c) { c.motor.l_uh = 9000000; c.stage.vbus_nominal_mv = 1000; c.stage.vbus_min_mv = 0; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"Kb above 32767", [](sstep_config& c) { c.motor.r_mohm = 9000000; c.motor.l_uh = 1; c.ctrl.fast_loop_hz = 1000; c.ctrl.current_bw_hz = 100; }, SSTEP_ERR_CONFIG},
        {"R pu above 32767", [](sstep_config& c) { c.motor.r_mohm = 9000000; c.stage.vbus_nominal_mv = 1; c.stage.vbus_min_mv = 0; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"ADC V gain above 32767", [](sstep_config& c) { c.stage.adc_vbus_mv_full = 9000000; c.stage.adc_full = 1; c.stage.adc_i_zero = 1; c.stage.vbus_nominal_mv = 1; c.stage.vbus_min_mv = 0; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"FF above 32767", [](sstep_config& c) { c.motor.ke_uv_s_per_rad = 9000000; c.motor.full_steps_per_rev = 1; c.ctrl.microsteps = 1; c.stage.vbus_nominal_mv = 1; c.stage.vbus_min_mv = 0; c.stage.vbus_max_mv = 0; }, SSTEP_ERR_CONFIG},
        {"ADC I gain", [](sstep_config& c) { c.stage.adc_i_span = 1; c.stage.i_trip_ma = 4000; }, SSTEP_OK},
    };
    // clang-format on
    for (const Case& k : cases)
    {
        Hal hal;
        sstep drv{};
        sstep_config cfg = make_config(&hal);
        k.mod(cfg);
        EXPECT_EQ(sstep_init(&drv, &cfg), k.expect) << k.name;
    }
}

TEST(SstepApi, InvertedCurrentAmplifier)
{
    /* negative span: positive current gives counts below zero */
    Hal hal;
    sstep drv{};
    sstep_config cfg = make_config(&hal);
    cfg.stage.adc_i_span = -2047;
    ASSERT_EQ(sstep_init(&drv, &cfg), SSTEP_OK);
    sstep_sample s{static_cast<uint16_t>(2048 - 1024), 2048, 2863};
    ASSERT_EQ(sstep_fast_loop(&drv, &s), SSTEP_OK);
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&drv, &st), SSTEP_OK);
    EXPECT_NEAR(st.i_a_ma, 2001, 2);
    EXPECT_NEAR(st.vbus_mv, 24000, 10);
}

/* ==============================================================================
 * STATES AND COMMANDS
 * =============================================================================*/

TEST(SstepApi, NullArguments)
{
    sstep_sample s{};
    sstep_status st{};
    EXPECT_EQ(sstep_enable(nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_disable(nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_fast_loop(nullptr, &s), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_slow_loop(nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_move_to(nullptr, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_move_by(nullptr, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_run(nullptr, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_stop(nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_position(nullptr, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_current(nullptr, 1, 1), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(nullptr, 1, 1, 1), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_report_fault(nullptr, 1), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_clear_faults(nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_get_status(nullptr, &st), SSTEP_ERR_PARAM);
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sstep_fast_loop(&sim.drv, nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_get_status(&sim.drv, nullptr), SSTEP_ERR_PARAM);
}

/* every function but sstep_init() refuses a zeroed instance (empty configuration, NULL
   callbacks) instead of dividing by zero or calling through a NULL pointer */
TEST(SstepApi, CallsBeforeInit)
{
    sstep drv{};
    sstep_sample s{2048, 2048, 2863};
    sstep_status st{};
    EXPECT_EQ(sstep_enable(&drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_disable(&drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_fast_loop(&drv, &s), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_slow_loop(&drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_move_to(&drv, 0), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_move_by(&drv, 0), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_run(&drv, 0), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_stop(&drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_set_position(&drv, 0), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_set_current(&drv, 1000, 500), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_set_motion_limits(&drv, 1000, 1000, 0), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_report_fault(&drv, SSTEP_FAULT_EXTERNAL), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_clear_faults(&drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_get_status(&drv, &st), SSTEP_ERR_STATE);
}

/* a failed sstep_init() after a valid one leaves the instance unusable, not half-derived */
TEST(SstepApi, FailedInitInvalidatesTheInstance)
{
    Hal hal;
    sstep drv{};
    sstep_config cfg = make_config(&hal);
    ASSERT_EQ(sstep_init(&drv, &cfg), SSTEP_OK);
    cfg.motor.l_uh = 9000000; /* Kp does not fit: fails in the derivation */
    cfg.stage.vbus_nominal_mv = 1000;
    cfg.stage.vbus_min_mv = 0;
    cfg.stage.vbus_max_mv = 0;
    ASSERT_EQ(sstep_init(&drv, &cfg), SSTEP_ERR_CONFIG);
    EXPECT_EQ(sstep_enable(&drv), SSTEP_ERR_STATE);
    EXPECT_FALSE(hal.enabled);
}

TEST(SstepApi, CommandsNeedEnabledOutputs)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sstep_move_to(&sim.drv, 10), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_move_by(&sim.drv, 10), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_run(&sim.drv, 100), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_stop(&sim.drv), SSTEP_OK); /* nothing to stop */
    sim.run_for(0.01);                         /* slow loop while disabled */
    EXPECT_EQ(sim.hal.pwm_writes, 0);
}

TEST(SstepApi, EnableDisable)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    EXPECT_TRUE(sim.hal.enabled);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_OK); /* already enabled */
    sim.run_for(0.002);
    EXPECT_GT(sim.hal.pwm_writes, 0);
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_HOLDING);
    ASSERT_EQ(sstep_disable(&sim.drv), SSTEP_OK);
    EXPECT_FALSE(sim.hal.enabled);
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_DISABLED);
}

TEST(SstepApi, ArgumentRanges)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    EXPECT_EQ(sstep_run(&sim.drv, 16001), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_run(&sim.drv, -16001), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_current(&sim.drv, 0, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_current(&sim.drv, 20000000, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_current(&sim.drv, 1000, 20000000), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_current(&sim.drv, 1000, 1500), SSTEP_ERR_CONFIG);
    EXPECT_EQ(sstep_set_current(&sim.drv, 2500, 0), SSTEP_ERR_CONFIG); /* > rated */
    EXPECT_EQ(sstep_set_current(&sim.drv, 1500, 500), SSTEP_OK);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 0, 1000, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 1000, 0, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 1000, 1000, 1),
              SSTEP_ERR_PARAM); /* dec too small */
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 2000000000, 1000, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 8000, 80000, 40000), SSTEP_OK);
    ASSERT_EQ(sstep_move_to(&sim.drv, INT32_MAX), SSTEP_OK);
    EXPECT_EQ(sstep_move_by(&sim.drv, 1), SSTEP_ERR_PARAM); /* overflow */
    ASSERT_EQ(sstep_move_to(&sim.drv, INT32_MIN), SSTEP_OK);
    EXPECT_EQ(sstep_move_by(&sim.drv, -1), SSTEP_ERR_PARAM);
}

TEST(SstepApi, AccelerationRangeAtLowSlowRate)
{
    /* slow loop at 1 Hz: accelerations above 16383 usteps/s^2 exceed 2^30 per period^2 */
    Sim sim;
    sim.cfg.ctrl.slow_loop_hz = 1U;
    sim.cfg.ctrl.accel = 1000U;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 1000, 20000, 0), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 1000, 1000, 20000), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_set_motion_limits(&sim.drv, 1000, 1000, 2000), SSTEP_OK);
}

TEST(SstepApi, SetPositionKeepsTheRotor)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_move_to(&sim.drv, 37), SSTEP_OK);
    sim.run_for(0.05);
    EXPECT_EQ(sstep_set_position(&sim.drv, 5), SSTEP_OK); /* standstill */
    const double th = sim.x.th;
    sim.run_for(0.02);
    EXPECT_NEAR(sim.x.th, th, 1e-3); /* no jump */
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.position, 5);
    EXPECT_EQ(st.target, 5);
    ASSERT_EQ(sstep_move_by(&sim.drv, 10), SSTEP_OK);
    sim.run_for(0.002);
    EXPECT_EQ(sstep_set_position(&sim.drv, 0), SSTEP_ERR_STATE); /* moving */
    sim.run_for(0.1);
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.position, 15);
    EXPECT_NEAR(sim.rotor_usteps(), 37 + 10, 0.5); /* rotor moved 10 more microsteps */
}

/* ==============================================================================
 * FAULTS
 * =============================================================================*/

TEST(SstepApi, OvercurrentTrips)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sstep_sample s = sim.sample();
    s.i_b = 2048 - 1900; /* -3.7 A > 3.5 A trip */
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    sstep_status st{};
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_FAULT);
    EXPECT_EQ(st.faults, SSTEP_FAULT_OVERCURRENT);
    EXPECT_FALSE(sim.hal.enabled);
    EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_OVERCURRENT);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_STATE);
    EXPECT_EQ(sstep_move_to(&sim.drv, 1), SSTEP_ERR_STATE);
    ASSERT_EQ(sstep_disable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_FAULT); /* disable does not clear a fault */
    ASSERT_EQ(sstep_clear_faults(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    EXPECT_EQ(st.state, SSTEP_STATE_DISABLED);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
}

TEST(SstepApi, OvercurrentPositiveAndPhaseA)
{
    for (int k = 0; k < 3; k++)
    {
        Sim sim;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sstep_sample s = sim.sample();
        if (k == 0)
        {
            s.i_a = 2048 + 1900;
        }
        if (k == 1)
        {
            s.i_a = 2048 - 1900;
        }
        if (k == 2)
        {
            s.i_b = 2048 + 1900;
        }
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
        EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_OVERCURRENT) << k;
    }
}

TEST(SstepApi, BusVoltageTrips)
{
    struct Case
    {
        double v;
        uint16_t fault;
    };
    for (const Case& c : {Case{31.0, SSTEP_FAULT_OVERVOLTAGE}, Case{9.0, SSTEP_FAULT_UNDERVOLTAGE}})
    {
        Sim sim;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.vbus = c.v;
        sim.run_for(0.0001);
        EXPECT_EQ(sim.hal.faults, c.fault);
        EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
    }
    /* disabled thresholds: no trip */
    Sim sim;
    sim.cfg.stage.vbus_min_mv = 0;
    sim.cfg.stage.vbus_max_mv = 0;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.vbus = 5.0;
    sim.run_for(0.001);
    sim.vbus = 33.0;
    sim.run_for(0.001);
    EXPECT_EQ(sim.hal.faults, 0U);
}

TEST(SstepApi, NoOvercurrentCheckInVoltageDrive)
{
    Sim sim;
    sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sstep_sample s = sim.sample();
    s.i_a = 4095;
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(sim.hal.faults, 0U);
}

TEST(SstepApi, ReportedFault)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    EXPECT_EQ(sstep_report_fault(&sim.drv, 0U), SSTEP_OK); /* nothing */
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_HOLDING);
    EXPECT_EQ(sstep_report_fault(&sim.drv, SSTEP_FAULT_EXTERNAL), SSTEP_OK);
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
    EXPECT_FALSE(sim.hal.enabled);
    EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_EXTERNAL);
}

TEST(SstepApi, FaultWithoutCallbacks)
{
    Sim sim;
    sim.cfg.hal.on_fault = nullptr;
    sim.cfg.hal.on_event = nullptr;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_move_to(&sim.drv, 20), SSTEP_OK);
    sim.run_for(0.2); /* move done and hold current without event callback */
    EXPECT_EQ(sstep_report_fault(&sim.drv, SSTEP_FAULT_EXTERNAL), SSTEP_OK);
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
}

TEST(SstepApi, HardwareErrors)
{
    {
        Sim sim; /* PWM write fails */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.hal.pwm_result = -5;
        sim.run_for(0.0001);
        EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_HAL);
        EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
    }
    {
        Sim sim; /* enabling the outputs fails */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.hal.enable_result = -1;
        EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_STATE);
        EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_HAL);
    }
    {
        Sim sim; /* disabling the outputs fails */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.hal.enable_result = -1;
        EXPECT_EQ(sstep_disable(&sim.drv), SSTEP_OK);
        EXPECT_EQ(sim.hal.faults, SSTEP_FAULT_HAL);
        EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
    }
}

TEST(SstepApi, FaultDuringSlowLoopIsKept)
{
    /* a fault raised between the motion update and the state update must win */
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.ctx().faults = SSTEP_FAULT_EXTERNAL; /* as if set by an interrupt */
    ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_FAULT);
}

TEST(SstepApi, SlowLoopLateHoldsTheAngle)
{
    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_run(&sim.drv, 8000), SSTEP_OK);
    sim.run_for(0.05);
    /* stop calling the slow loop: the angle advances one slow period, then stays */
    sstep_sample s = sim.sample();
    for (int k = 0; k < 20; k++)
    {
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    }
    const sstep_angle_t a = sim.ctx().cur.angle;
    for (int k = 0; k < 40; k++)
    {
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    }
    EXPECT_EQ(sim.ctx().cur.angle, a);
}

TEST(SstepApi, BusVoltageZeroKeepsLastScale)
{
    Sim sim;
    sim.cfg.stage.vbus_min_mv = 0;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sim.vbus = 0.0;
    sim.run_for(0.003);
    EXPECT_EQ(sim.ctx().state, SSTEP_STATE_HOLDING);
}

TEST(SstepApi, NoBackEmfConstant)
{
    /* ke = 0: no feed-forward, the drive still works */
    Sim sim;
    sim.cfg.motor.ke_uv_s_per_rad = 0U;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_run(&sim.drv, 3200), SSTEP_OK);
    sim.run_for(0.1);
    EXPECT_EQ(sim.ctx().cur.ff, SSTEP_REAL_ZERO);
    EXPECT_LT(std::fabs(sim.load_angle_deg()), 90.0);
}

TEST(SstepApi, BusVoltageThresholdsBeyondAdcRange)
{
    /* thresholds above the ADC range are limited to the largest reading */
    Sim sim;
    sim.cfg.stage.adc_vbus_mv_full = 100U; /* 100 mV at full scale */
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sim.ctx().vbus_max_raw, 65535U);
    EXPECT_EQ(sim.ctx().vbus_min_raw, 65535U);
}

TEST(SstepApi, BusVoltageScaleOutOfRangeKeepsLast)
{
    /* V_base / Vbus above 32767 cannot be represented: the last scale is kept */
    Sim sim;
    sim.cfg.stage.adc_vbus_mv_full = 1U;
    sim.cfg.stage.vbus_min_mv = 0U;
    sim.cfg.stage.vbus_max_mv = 0U;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    const sstep_gain before = sim.ctx().inv_vbus;
    sstep_sample s = sim.sample();
    s.vbus = 1U;
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    EXPECT_EQ(std::memcmp(&before, &sim.ctx().inv_vbus, sizeof(before)), 0);
}

TEST(SstepApi, FeedForwardSaturates)
{
    /* very large back-EMF per speed: the feed-forward amplitude saturates at 1 pu */
    Sim sim;
    sim.cfg.motor.ke_uv_s_per_rad = 9000000U;
    sim.cfg.motor.full_steps_per_rev = 4U;
    sim.cfg.ctrl.microsteps = 1U;
    sim.cfg.ctrl.v_max = 1000U;
    sim.cfg.ctrl.accel = 100000U;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_run(&sim.drv, 500), SSTEP_OK);
    for (int k = 0; k < 10; k++)
    {
        ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    }
    sstep_sample s = sim.sample();
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(sim.ctx().cur.ff, SSTEP_REAL_MAX);
    ASSERT_EQ(sstep_run(&sim.drv, -500), SSTEP_OK);
    for (int k = 0; k < 30; k++)
    {
        ASSERT_EQ(sstep_slow_loop(&sim.drv), SSTEP_OK);
    }
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(sim.ctx().cur.ff, SSTEP_REAL_MIN);
}
