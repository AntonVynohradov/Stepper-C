/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Self-test, bus voltage check of the enable and the extra overcurrent trips.
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
#include <cstdio>
#include <cstdlib>

#include "sstep_test_plant.hpp"

using sstep_test::Hal;
using sstep_test::kSpan;
using sstep_test::kVFull;
using sstep_test::kZero;
using sstep_test::make_config;
using sstep_test::Sim;

namespace
{

#if (SSTEP_CFG_SELFTEST == 1)
/* 12 V through the divider of the test configuration */
constexpr uint16_t kVbus12 = 1432U;

/* ==============================================================================
 * INTEGER RL PLANT FOR THE SELF-TEST LOGIC
 * =============================================================================*/

/* A phase between legs p and q carries i from p to q (1.6 Ohm, 3 mH, 12 V, 20 kHz). The dead
   time eats loss_mv of a driving leg. Currents in counts of the test configuration. */
struct Phase
{
    unsigned p, q;
    int64_t r_mohm = 1600, l_uh = 3000;
    bool open = false;
    int64_t i_ua = 0;
};

struct Plant
{
    Phase ph[2];
    bool invert = false;    /* current sense of the wrong sign */
    int32_t leak_ma = 0;    /* constant current in phase A */
    int32_t induced_ma = 0; /* in the idle phase while the other is driven (rotor moving) */
    int64_t loss_mv = 576;  /* 2 * 1.2 us * 20 kHz * 12 V */
    uint16_t last[SSTEP_LEGS_MAX] = {0, 0, 0, 0};

    explicit Plant(sstep_topology topo)
    {
        ph[0].p = 0;
        ph[0].q = (topo == SSTEP_TOPOLOGY_FULL_BRIDGE) ? 1U : 2U;
        ph[1].p = (topo == SSTEP_TOPOLOGY_FULL_BRIDGE) ? 2U : 1U;
        ph[1].q = (topo == SSTEP_TOPOLOGY_FULL_BRIDGE) ? 3U : 2U;
    }

    int32_t counts(int64_t i_ua) const
    {
        const auto c = static_cast<int32_t>(i_ua * kSpan / 4000000);
        return invert ? -c : c;
    }

    void currents(int32_t* ia, int32_t* ib) const
    {
        int64_t i[2] = {ph[0].i_ua + leak_ma * 1000LL, ph[1].i_ua};
        for (unsigned k = 0; k < 2; k++)
        {
            if (last[ph[1 - k].p] > 0U)
            {
                i[k] += induced_ma * 1000LL;
            }
        }
        *ia = counts(i[0]);
        *ib = counts(i[1]);
    }

    void period(const uint16_t cmp[SSTEP_LEGS_MAX])
    {
        std::copy(cmp, cmp + SSTEP_LEGS_MAX, last);
        for (Phase& f : ph)
        {
            const int64_t drive = (static_cast<int64_t>(cmp[f.p]) - cmp[f.q]) * 12000 / 4250;
            int64_t v_mv = 0;
            if (f.open)
            {
                f.i_ua = 0;
                continue;
            }
            if (drive > loss_mv)
            {
                v_mv = drive - loss_mv;
            }
            else if (drive < -loss_mv)
            {
                v_mv = drive + loss_mv;
            }
            const int64_t v_uv = v_mv * 1000 - f.r_mohm * f.i_ua / 1000;
            f.i_ua += v_uv * 50 / f.l_uh;
        }
    }
};

sstep_config esc1(Hal* hal, sstep_topology topo = SSTEP_TOPOLOGY_SHARED_LEG)
{
    sstep_config c = make_config(hal, topo);
    c.stage.dead_time_ns = 1200U;
    return c;
}

sstep_selftest_result run(sstep_selftest_ctx* st, Plant* pl, uint32_t max_periods = 4000U)
{
    uint16_t cmp[SSTEP_LEGS_MAX] = {0, 0, 0, 0};
    sstep_selftest_result r = SSTEP_SELFTEST_RUNNING;
    for (uint32_t n = 0; (n < max_periods) && (r == SSTEP_SELFTEST_RUNNING); n++)
    {
        int32_t ia = 0;
        int32_t ib = 0;
        pl->currents(&ia, &ib);
        r = sstep_selftest_step(st, ia, ib, cmp);
        pl->period(cmp);
    }
    return r;
}

sstep_selftest_result run_case(Plant pl, sstep_topology topo = SSTEP_TOPOLOGY_SHARED_LEG,
                               sstep_selftest_ctx* out = nullptr)
{
    Hal hal;
    const sstep_config c = esc1(&hal, topo);
    sstep_selftest_ctx st{};
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    const sstep_selftest_result r = run(&st, &pl);
    if (out != nullptr)
    {
        *out = st;
    }
    return r;
}

#endif /* SSTEP_CFG_SELFTEST */

sstep_status status_of(Sim& sim)
{
    sstep_status st{};
    EXPECT_EQ(sstep_get_status(&sim.drv, &st), SSTEP_OK);
    return st;
}

#if (SSTEP_CFG_SELFTEST == 1)
/* Run the fast and slow loop until the self-test has ended (and one slow period more) */
void run_test(Sim& sim)
{
    for (int k = 0; (k < 4000) && (sim.ctx().st.result == SSTEP_SELFTEST_RUNNING); k++)
    {
        sim.step();
    }
    sim.run_for(0.002);
}
#endif /* SSTEP_CFG_SELFTEST */

}  // namespace

#if (SSTEP_CFG_SELFTEST == 1)

/* ==============================================================================
 * SELF-TEST LOGIC
 * =============================================================================*/

TEST(SstepSelftest, ParametersAt12V)
{
    Hal hal;
    const sstep_config c = esc1(&hal);
    sstep_selftest_ctx st{};
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    const int64_t vbus_mv = int64_t{kVbus12} * 34320 / 4095;
    const int64_t loss = 2LL * 1200 * 20000 * 4250 / 1000000000LL; /* 204 ticks */
    const int64_t duty_max = 3LL * 960 * 4250 / vbus_mv + loss;
    EXPECT_EQ(st.i_test, 600 * kSpan / 4000);
    EXPECT_EQ(st.duty_max_q8, static_cast<uint32_t>(duty_max * 256));
    EXPECT_EQ(st.duty_short_q8, static_cast<uint32_t>((960LL * 4250 / (4 * vbus_mv) + loss) * 256));
    EXPECT_EQ(st.ramp_q8, static_cast<uint32_t>(duty_max * 256 / 400)); /* 20 ms */
    EXPECT_EQ(st.boot_periods, 40U);
    EXPECT_EQ(st.hold_periods, 40U);
    EXPECT_EQ(st.decay_periods, 100U);
    EXPECT_EQ(st.leg[1], 1U);
    EXPECT_EQ(st.result, SSTEP_SELFTEST_RUNNING);
    const sstep_config f = esc1(&hal, SSTEP_TOPOLOGY_FULL_BRIDGE);
    sstep_selftest_setup(&st, &f, kVbus12, 600U);
    EXPECT_EQ(st.leg[1], 2U);
}

TEST(SstepSelftest, ParameterLimits)
{
    Hal hal;
    sstep_config c = esc1(&hal);
    sstep_selftest_ctx st{};

    c.motor.r_mohm = 1000000U; /* duty_max beyond the usable range */
    c.stage.sample_window_ticks = 400U;
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    EXPECT_EQ(st.duty_max_q8, (4250U - 400U) * 256U);

    c = esc1(&hal); /* duty_short beyond duty_max: a dead time of 20 us, a window */
    c.stage.dead_time_ns = 20000U;
    c.stage.sample_window_ticks = 1062U;
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    EXPECT_EQ(st.duty_short_q8, st.duty_max_q8);

    c = esc1(&hal); /* no voltage, no dead time, 1 mA: everything at its floor */
    c.stage.dead_time_ns = 0U;
    c.motor.r_mohm = 1U;
    c.ctrl.fast_loop_hz = 100U;
    sstep_selftest_setup(&st, &c, 0U, 1U);
    EXPECT_EQ(st.i_test, 1);
    EXPECT_EQ(st.duty_max_q8, 256U);
    EXPECT_EQ(st.duty_short_q8, 0U);
    EXPECT_EQ(st.ramp_q8, 128U);
    EXPECT_EQ(st.boot_periods, 1U);
    sstep_selftest_setup(&st, &c, kVbus12, 1U); /* ramp below one tick/256 per period */
    c.ctrl.fast_loop_hz = 20000U;
    sstep_selftest_setup(&st, &c, kVbus12, 1U);
    EXPECT_EQ(st.ramp_q8, 1U);
}

TEST(SstepSelftest, HealthyMotorPasses)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_SHARED_LEG, SSTEP_TOPOLOGY_FULL_BRIDGE})
    {
        sstep_selftest_ctx st{};
        EXPECT_EQ(run_case(Plant(topo), topo, &st), SSTEP_SELFTEST_PASS);
        EXPECT_EQ(st.coil, 1U);
        EXPECT_GE(st.i_own, st.i_test);
        uint16_t cmp[SSTEP_LEGS_MAX] = {1, 1, 1, 1};
        EXPECT_EQ(sstep_selftest_step(&st, 0, 0, cmp), SSTEP_SELFTEST_PASS); /* stays */
        EXPECT_EQ(cmp[0] + cmp[1] + cmp[2] + cmp[3], 0);
    }
}

TEST(SstepSelftest, OpenPhase)
{
    for (unsigned k = 0; k < 2; k++)
    {
        Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
        pl.ph[k].open = true;
        sstep_selftest_ctx st{};
        EXPECT_EQ(run_case(pl, SSTEP_TOPOLOGY_SHARED_LEG, &st), SSTEP_SELFTEST_OPEN);
        EXPECT_EQ(st.coil, k);
        EXPECT_GT(st.duty_q8, st.duty_max_q8);
    }
}

TEST(SstepSelftest, ShortCircuit)
{
    Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
    pl.ph[0].r_mohm = 20;
    pl.ph[0].l_uh = 1;
    EXPECT_EQ(run_case(pl), SSTEP_SELFTEST_SHORT);
}

TEST(SstepSelftest, PhaseOnTheWrongLegs)
{
    Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
    pl.ph[0].q = 1; /* phase A between U and V */
    EXPECT_EQ(run_case(pl), SSTEP_SELFTEST_WRONG_PHASE);
}

TEST(SstepSelftest, WrongCurrentSign)
{
    Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
    pl.invert = true;
    EXPECT_EQ(run_case(pl), SSTEP_SELFTEST_POLARITY);
}

TEST(SstepSelftest, LeakingSwitch)
{
    Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
    pl.leak_ma = 400;
    EXPECT_EQ(run_case(pl), SSTEP_SELFTEST_LEAK);
}

TEST(SstepSelftest, CurrentInducedByTheRotorPasses)
{
    Plant pl(SSTEP_TOPOLOGY_SHARED_LEG);
    pl.induced_ma = -240; /* 40 % of the test current in the idle phase */
    EXPECT_EQ(run_case(pl), SSTEP_SELFTEST_PASS);
}

TEST(SstepSelftest, CurrentAboveTwiceTheTestCurrentIsAShort)
{
    Hal hal;
    const sstep_config c = esc1(&hal);
    sstep_selftest_ctx st{};
    uint16_t cmp[SSTEP_LEGS_MAX];
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    EXPECT_EQ(sstep_selftest_step(&st, 0, 2 * st.i_test + 1, cmp), SSTEP_SELFTEST_SHORT);
    EXPECT_EQ(st.i_own, 0);
}

TEST(SstepSelftest, SmallTestCurrent)
{
    Hal hal;
    const sstep_config c = esc1(&hal);
    sstep_selftest_ctx st{};
    uint16_t cmp[SSTEP_LEGS_MAX];
    sstep_selftest_setup(&st, &c, kVbus12, 5U); /* 2 counts: the leak level is 1 count */
    ASSERT_LT(st.i_test, 4);
    sstep_selftest_result r = SSTEP_SELFTEST_RUNNING;
    for (uint32_t n = 0; n < st.boot_periods; n++)
    {
        r = sstep_selftest_step(&st, 0, (n + 1U == st.boot_periods) ? 2 : 0, cmp);
    }
    EXPECT_EQ(r, SSTEP_SELFTEST_LEAK);
}

/* ==============================================================================
 * SELF-TEST THROUGH THE API, ON THE MOTOR MODEL
 * =============================================================================*/

TEST(SstepSelftest, PassesOnTheMotorModel)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_SHARED_LEG, SSTEP_TOPOLOGY_FULL_BRIDGE})
    {
        Sim sim(topo);
        sim.vbus = 12.0;
        sim.cfg.stage.dead_time_ns = 1200U;
        sim.v_dead = 0.29; /* 1.2 us of 50 us at 12 V */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001); /* a bus voltage sample */
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        EXPECT_TRUE(sim.hal.enabled);
        const double t0 = sim.rec.empty() ? 0.0 : sim.rec.back().t;
        int k = 0;
        for (; (k < 4000) && (sim.ctx().st.result == SSTEP_SELFTEST_RUNNING); k++)
        {
            sim.step();
        }
        sim.run_for(0.002);
        const sstep_status st = status_of(sim);
        std::printf("[ measured ] self-test %s, free rotor: %s after %.1f ms, phase B %d mA at "
                    "compare %u\n",
                    topo == SSTEP_TOPOLOGY_FULL_BRIDGE ? "full" : "shared",
                    st.selftest == SSTEP_SELFTEST_PASS ? "pass" : "FAIL", k * 0.05,
                    st.selftest_i_ma, st.selftest_duty);
        (void)t0;
        EXPECT_EQ(st.selftest, SSTEP_SELFTEST_PASS);
        EXPECT_EQ(st.state, SSTEP_STATE_DISABLED);
        EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
        EXPECT_EQ(st.selftest_phase, 1U);
        EXPECT_GE(st.selftest_i_ma, 600); /* reached, held below twice the test current */
        EXPECT_LT(st.selftest_i_ma, 1200);
        EXPECT_GT(st.selftest_duty, 0U);
        EXPECT_FALSE(sim.hal.enabled);
        EXPECT_NE(
            std::find(sim.hal.events.begin(), sim.hal.events.end(), SSTEP_EVENT_SELFTEST_DONE),
            sim.hal.events.end());
        EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    }
}

TEST(SstepSelftest, FindsAnOpenPhaseOnTheMotorModel)
{
    Sim sim;
    sim.open_a = true;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    sim.run_for(0.001);
    ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
    run_test(sim);
    const sstep_status st = status_of(sim);
    EXPECT_EQ(st.selftest, SSTEP_SELFTEST_OPEN);
    EXPECT_EQ(st.selftest_phase, 0U);
    EXPECT_EQ(st.state, SSTEP_STATE_FAULT);
    EXPECT_EQ(st.faults, SSTEP_FAULT_SELFTEST);
    EXPECT_FALSE(sim.hal.enabled);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_STATE);
    ASSERT_EQ(sstep_clear_faults(&sim.drv), SSTEP_OK);
    EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_OPEN); /* kept for the application */
}

TEST(SstepSelftest, Arguments)
{
    Hal hal;
    sstep zeroed{};
    EXPECT_EQ(sstep_selftest(nullptr, 600U), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_selftest(&zeroed, 600U), SSTEP_ERR_STATE);

    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_VBUS); /* no sample yet */
    sim.run_for(0.001);
    EXPECT_EQ(sstep_selftest(&sim.drv, 0U), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_selftest(&sim.drv, 2001U), SSTEP_ERR_PARAM); /* above i_run_ma */
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_STATE);
    ASSERT_EQ(sstep_disable(&sim.drv), SSTEP_OK);

    ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
    EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_STATE); /* running */
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_STATE);
    ASSERT_EQ(sstep_disable(&sim.drv), SSTEP_OK); /* stops it */
    EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_NONE);
    EXPECT_FALSE(sim.hal.enabled);
}

TEST(SstepSelftest, CallbackErrors)
{
    {
        Sim sim; /* outputs refused at the start */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        sim.hal.enable_result = -1;
        EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_STATE);
        EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_ABORTED);
        EXPECT_EQ(status_of(sim).faults, SSTEP_FAULT_HAL);
    }
    {
        Sim sim; /* PWM write fails during the test */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        sim.hal.pwm_result = -1;
        sim.run_for(0.001);
        EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_ABORTED);
        EXPECT_EQ(status_of(sim).faults, SSTEP_FAULT_HAL);
    }
    {
        Sim sim; /* outputs cannot be switched off at the end */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        sim.hal.enable_result = -1;
        run_test(sim);
        EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_PASS);
        EXPECT_EQ(status_of(sim).faults, SSTEP_FAULT_HAL);
    }
}

TEST(SstepSelftest, TripsDuringTheTest)
{
    {
        Sim sim; /* overcurrent: a short */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        sstep_sample s = sim.sample();
        s.i_b = static_cast<uint16_t>(kZero - 1900); /* -3.7 A */
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
        const sstep_status st = status_of(sim);
        EXPECT_EQ(st.selftest, SSTEP_SELFTEST_SHORT);
        EXPECT_EQ(st.faults, SSTEP_FAULT_OVERCURRENT | SSTEP_FAULT_SELFTEST);
    }
    {
        Sim sim; /* undervoltage: aborted, no self-test fault */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        sstep_sample s = sim.sample();
        s.vbus = static_cast<uint16_t>(8.0 / kVFull * 4095.0);
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
        const sstep_status st = status_of(sim);
        EXPECT_EQ(st.selftest, SSTEP_SELFTEST_ABORTED);
        EXPECT_EQ(st.faults, SSTEP_FAULT_UNDERVOLTAGE);
        sim.run_for(0.002);
        EXPECT_NE(
            std::find(sim.hal.events.begin(), sim.hal.events.end(), SSTEP_EVENT_SELFTEST_DONE),
            sim.hal.events.end());
    }
}

TEST(SstepSelftest, EventAfterAnEnableAndWithoutCallback)
{
    {
        Sim sim; /* enabled before the slow loop reported the end: reported while active */
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        for (int k = 0; (k < 4000) && (sim.ctx().st.result == SSTEP_SELFTEST_RUNNING); k++)
        {
            sim.step();
        }
        ASSERT_EQ(sim.ctx().st.done, 1U);
        ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
        sim.run_for(0.002);
        EXPECT_EQ(sim.ctx().st.done, 0U);
        EXPECT_NE(
            std::find(sim.hal.events.begin(), sim.hal.events.end(), SSTEP_EVENT_SELFTEST_DONE),
            sim.hal.events.end());
    }
    {
        Sim sim;
        sim.cfg.hal.on_event = nullptr;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sim.run_for(0.001);
        ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
        run_test(sim);
        EXPECT_EQ(sim.ctx().st.done, 0U);
        EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_PASS);
    }
}

TEST(SstepSelftest, WrongSignOfTheSpanIsFound)
{
    Sim sim; /* the amplifier is not inverting, the configuration says it is */
    sim.cfg.stage.adc_i_span = -kSpan;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    sim.run_for(0.001);
    ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
    run_test(sim);
    EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_POLARITY);
    EXPECT_EQ(status_of(sim).faults, SSTEP_FAULT_SELFTEST);
}

TEST(SstepSelftest, RunsAndTripsInVoltageDrive)
{
    Sim sim; /* the test checks the currents even when the control does not */
    sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    sim.run_for(0.001);
    ASSERT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_OK);
    sstep_sample s = sim.sample();
    s.i_a = static_cast<uint16_t>(kZero + 1900);
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(status_of(sim).selftest, SSTEP_SELFTEST_SHORT);
}

TEST(SstepSelftest, PhaseAAboveTwiceTheTestCurrentIsAShort)
{
    Hal hal;
    const sstep_config c = esc1(&hal);
    sstep_selftest_ctx st{};
    uint16_t cmp[SSTEP_LEGS_MAX];
    sstep_selftest_setup(&st, &c, kVbus12, 600U);
    EXPECT_EQ(sstep_selftest_step(&st, -2 * st.i_test - 1, 0, cmp), SSTEP_SELFTEST_SHORT);
}

#else /* SSTEP_CFG_SELFTEST == 0 */

/* ==============================================================================
 * SELF-TEST COMPILED OUT
 * =============================================================================*/

TEST(SstepSelftest, CompiledOut)
{
    sstep zeroed{};
    EXPECT_EQ(sstep_selftest(nullptr, 600U), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_selftest(&zeroed, 600U), SSTEP_ERR_STATE);

    Sim sim;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    sim.run_for(0.001);
    EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_UNSUPPORTED);
    EXPECT_FALSE(sim.hal.enabled);

    /* enable, move and disable work as with the self-test */
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    ASSERT_EQ(sstep_move_by(&sim.drv, 64), SSTEP_OK);
    sim.run_for(0.05);
    const sstep_status st = status_of(sim);
    EXPECT_EQ(st.position, 64);
    EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
    EXPECT_EQ(st.selftest, SSTEP_SELFTEST_NONE);
    EXPECT_EQ(st.selftest_phase, 0U);
    EXPECT_EQ(st.selftest_i_ma, 0);
    EXPECT_EQ(st.selftest_duty, 0U);
    ASSERT_EQ(sstep_disable(&sim.drv), SSTEP_OK);
    EXPECT_FALSE(sim.hal.enabled);
    EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_UNSUPPORTED);
}

#endif /* SSTEP_CFG_SELFTEST */

/* ==============================================================================
 * BUS VOLTAGE CHECK OF THE ENABLE
 * =============================================================================*/

TEST(SstepProtection, BusVoltageOutOfRange)
{
    for (double v : {8.0, 31.0})
    {
        Sim sim;
        sim.vbus = v;
        ASSERT_EQ(sim.init(), SSTEP_OK);
        sstep_sample s = sim.sample();
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
#if (SSTEP_CFG_SELFTEST == 1)
        EXPECT_EQ(sstep_selftest(&sim.drv, 600U), SSTEP_ERR_VBUS);
#endif
        EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_ERR_VBUS);
        const sstep_status st = status_of(sim);
        EXPECT_EQ(st.state, SSTEP_STATE_DISABLED); /* not latched */
        EXPECT_EQ(st.faults, SSTEP_FAULT_NONE);
        EXPECT_FALSE(sim.hal.enabled);
        sim.vbus = 24.0;
        s = sim.sample();
        ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
        EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    }
    /* trips switched off: any voltage is accepted */
    Sim sim;
    sim.vbus = 8.0;
    sim.cfg.stage.vbus_min_mv = 0U;
    sim.cfg.stage.vbus_max_mv = 0U;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    sstep_sample s = sim.sample();
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
}

/* ==============================================================================
 * OVERCURRENT OF THE SHARED LEG AND IN VOLTAGE DRIVE
 * =============================================================================*/

TEST(SstepProtection, SharedLegOvercurrent)
{
    for (sstep_topology topo : {SSTEP_TOPOLOGY_SHARED_LEG, SSTEP_TOPOLOGY_FULL_BRIDGE})
    {
        for (int sign : {1, -1})
        {
            Sim sim(topo);
            ASSERT_EQ(sim.init(), SSTEP_OK);
            ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
            sstep_sample s = sim.sample();
            /* 2 A in each phase, below the 3.5 A trip; 4 A in the shared leg */
            s.i_a = static_cast<uint16_t>(kZero + sign * 1024);
            s.i_b = static_cast<uint16_t>(kZero + sign * 1024);
            ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
            EXPECT_EQ(status_of(sim).faults, (topo == SSTEP_TOPOLOGY_SHARED_LEG) ?
                                                 SSTEP_FAULT_OVERCURRENT :
                                                 SSTEP_FAULT_NONE);
        }
    }
}

TEST(SstepProtection, OvercurrentInVoltageDriveWhenAsked)
{
    Sim sim;
    sim.cfg.ctrl.drive_mode = SSTEP_DRIVE_VOLTAGE;
    sim.cfg.ctrl.i_trip_voltage_drive = true;
    ASSERT_EQ(sim.init(), SSTEP_OK);
    ASSERT_EQ(sstep_enable(&sim.drv), SSTEP_OK);
    sstep_sample s = sim.sample();
    s.i_a = static_cast<uint16_t>(kZero + 1900);
    ASSERT_EQ(sstep_fast_loop(&sim.drv, &s), SSTEP_OK);
    EXPECT_EQ(status_of(sim).faults, SSTEP_FAULT_OVERCURRENT);
}
