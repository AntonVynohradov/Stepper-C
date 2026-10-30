/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Unit tests of the motion generator (integer trapezoidal profile).
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
#include <cstdint>
#include <cstdlib>

#include "sstep_test_plant.hpp"

/* ==============================================================================
 * HELPERS
 * =============================================================================*/

namespace
{

constexpr int64_t kOne = 65536;

/* limits of motion_default: 16000 usteps/s, 160000 usteps/s^2 at 1 kHz */
sstep_motion make_motion()
{
    sstep_motion mo{};
    mo.v_max = static_cast<int32_t>(16000 * kOne / 1000);
    mo.acc = static_cast<int32_t>(std::llround(160000.0 * kOne / 1e6));
    mo.dec = mo.acc;
    mo.brake_vmax = static_cast<int64_t>(mo.v_max) * mo.v_max / (2 * mo.dec);
    return mo;
}

struct Profile
{
    int periods = 0;
    int done_flags = 0;
    int32_t v_peak = 0;
    int32_t dv_max = 0;  /* largest velocity change, last period excluded */
    int32_t dv_last = 0; /* velocity change of the last period (stop on the target) */
    bool monotonic = true;
};

Profile run_move(sstep_motion& mo, int64_t target, int limit)
{
    Profile pr;
    mo.target = target * kOne;
    mo.mode = SSTEP_MODE_MOVE;
    const int64_t dir = (mo.target >= mo.pos) ? 1 : -1;
    while ((pr.periods < limit) && (mo.mode == SSTEP_MODE_MOVE))
    {
        const int64_t p0 = mo.pos;
        const int32_t v0 = mo.vel;
        if (sstep_motion_step(&mo) & SSTEP_MOTION_DONE)
        {
            pr.done_flags++;
        }
        pr.monotonic = pr.monotonic && ((mo.pos - p0) * dir >= 0);
        pr.v_peak = std::max(pr.v_peak, std::abs(mo.vel));
        if (mo.mode == SSTEP_MODE_MOVE)
        {
            pr.dv_max = std::max(pr.dv_max, std::abs(mo.vel - v0));
        }
        else
        {
            pr.dv_last = std::abs(mo.vel - v0);
        }
        pr.periods++;
    }
    return pr;
}

} // namespace

/* ==============================================================================
 * TESTS
 * =============================================================================*/

TEST(SstepMotion, IntegerSquareRoot)
{
    EXPECT_EQ(sstep_isqrt64(0U), 0U);
    EXPECT_EQ(sstep_isqrt64(1U), 1U);
    EXPECT_EQ(sstep_isqrt64(15U), 3U);
    EXPECT_EQ(sstep_isqrt64(16U), 4U);
    EXPECT_EQ(sstep_isqrt64(UINT64_MAX), 4294967295U);
    uint64_t s = 99U;
    for (int k = 0; k < 100000; k++)
    {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        const uint64_t x = s >> (s & 63U);
        const uint64_t r = sstep_isqrt64(x);
        ASSERT_LE(r * r, x);
        ASSERT_TRUE((r == 0xFFFFFFFFULL) || ((r + 1) * (r + 1) > x)) << x; /* (r+1)^2 > x */
    }
}

TEST(SstepMotion, MoveTwoRevolutions)
{
    /* T05: 6400 usteps (2 rev, M = 16), as in the Scilab prototype: 499 ms */
    sstep_motion mo = make_motion();
    const Profile pr = run_move(mo, 6400, 2000);
    EXPECT_EQ(mo.pos, 6400 * kOne); /* exactly on the target */
    EXPECT_EQ(mo.vel, 0);
    EXPECT_EQ(pr.done_flags, 1);
    EXPECT_TRUE(pr.monotonic);
    EXPECT_EQ(pr.v_peak, mo.v_max);
    EXPECT_LE(pr.dv_max, mo.acc);
    EXPECT_NEAR(pr.periods, 499, 2);
    std::printf("[ measured ] move 6400 usteps: %d ms, v peak %.1f usteps/s\n", pr.periods,
                pr.v_peak * 1000.0 / kOne);
}

TEST(SstepMotion, ShortAndNegativeMoves)
{
    for (int64_t target : {1LL, 3LL, 17LL, 160LL, -1LL, -250LL, 100000LL})
    {
        sstep_motion mo = make_motion();
        const Profile pr = run_move(mo, target, 20000);
        EXPECT_EQ(mo.pos, target * kOne) << target;
        EXPECT_EQ(pr.done_flags, 1) << target;
        EXPECT_TRUE(pr.monotonic) << target;
        EXPECT_LE(pr.dv_max, mo.acc) << target;
        EXPECT_LE(pr.dv_last, 2 * mo.dec) << target; /* final stop: below 2 dec */
    }
}

TEST(SstepMotion, RandomTargetsNoVelocityStep)
{
    /* every move ends exactly on target, monotonic, and the velocity never changes by
       more than acc / dec per period, including the last one */
    uint32_t s = 3U;
    for (int k = 0; k < 2000; k++)
    {
        s = s * 1664525U + 1013904223U;
        const int64_t target = static_cast<int64_t>(s % 20001U) - 10000;
        sstep_motion mo = make_motion();
        const Profile pr = run_move(mo, target, 100000);
        ASSERT_EQ(mo.pos, target * kOne) << target;
        ASSERT_TRUE(pr.monotonic) << target;
        ASSERT_LE(pr.dv_max, mo.acc) << target;
        ASSERT_LE(pr.dv_last, 2 * mo.dec) << target;
        ASSERT_LE(pr.v_peak, mo.v_max) << target;
        ASSERT_EQ(pr.done_flags, 1) << target;
    }
}

TEST(SstepMotion, MoveToCurrentPositionFinishesAtOnce)
{
    sstep_motion mo = make_motion();
    mo.pos = 42 * kOne;
    const Profile pr = run_move(mo, 42, 10);
    EXPECT_EQ(pr.periods, 1);
    EXPECT_EQ(pr.done_flags, 1);
}

TEST(SstepMotion, RetargetWhileMovingReverses)
{
    /* new target behind: decelerate with dec, never jump */
    sstep_motion mo = make_motion();
    mo.target = 6400 * kOne;
    mo.mode = SSTEP_MODE_MOVE;
    for (int k = 0; k < 150; k++)
    {
        (void)sstep_motion_step(&mo);
    }
    ASSERT_GT(mo.vel, 0);
    const Profile pr = run_move(mo, 0, 5000);
    EXPECT_EQ(mo.pos, 0);
    EXPECT_LE(pr.dv_max, mo.dec);
}

TEST(SstepMotion, RunAndReverse)
{
    sstep_motion mo = make_motion();
    mo.mode = SSTEP_MODE_RUN;
    mo.v_run = mo.v_max * 4; /* above the limit: clamped */
    int32_t dv = 0;
    for (int k = 0; k < 200; k++)
    {
        const int32_t v0 = mo.vel;
        (void)sstep_motion_step(&mo);
        dv = std::max(dv, std::abs(mo.vel - v0));
    }
    EXPECT_EQ(mo.vel, mo.v_max);
    mo.v_run = -mo.v_max;
    for (int k = 0; k < 300; k++)
    {
        const int32_t v0 = mo.vel;
        (void)sstep_motion_step(&mo);
        dv = std::max(dv, std::abs(mo.vel - v0));
    }
    EXPECT_EQ(mo.vel, -mo.v_max);
    EXPECT_LE(dv, mo.acc);
}

TEST(SstepMotion, HoldModeDoesNotMove)
{
    sstep_motion mo = make_motion();
    mo.pos = 5 * kOne;
    for (int k = 0; k < 10; k++)
    {
        EXPECT_EQ(sstep_motion_step(&mo), 0U);
    }
    EXPECT_EQ(mo.pos, 5 * kOne);
    EXPECT_EQ(mo.vel, 0);
}

TEST(SstepMotion, ReverseFromSlowForward)
{
    /* moving slowly forward, command a fast backward run: first brake with dec, then
       accelerate backwards with acc */
    sstep_motion mo = make_motion();
    mo.mode = SSTEP_MODE_RUN;
    mo.v_run = mo.acc * 3;
    for (int k = 0; k < 5; k++)
    {
        (void)sstep_motion_step(&mo);
    }
    ASSERT_EQ(mo.vel, mo.acc * 3);
    mo.v_run = -mo.v_max;
    for (int k = 0; k < 4; k++)
    {
        const int32_t v0 = mo.vel;
        (void)sstep_motion_step(&mo);
        EXPECT_EQ(mo.vel - v0, -mo.dec) << k;
    }
    EXPECT_EQ(mo.vel, -mo.acc);
}

TEST(SstepMotion, NearTargetWithSpeedDoesNotSnap)
{
    /* closer than the snap distance after this period but still faster than dec:
       continue braking instead of stopping abruptly */
    sstep_motion mo = make_motion();
    const int32_t d = mo.dec;
    mo.target = 100 * kOne;
    mo.vel = 3 * d;
    mo.pos = mo.target - 30 - (5 * static_cast<int64_t>(d)) / 2;
    mo.mode = SSTEP_MODE_MOVE;
    EXPECT_EQ(sstep_motion_step(&mo), 0U);
    EXPECT_EQ(mo.vel, 2 * d);
    EXPECT_EQ(mo.mode, SSTEP_MODE_MOVE);
    EXPECT_EQ(mo.target - mo.pos, 30);
}
