/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Unit tests of the numeric backend (fixed point and float builds).
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
#include <cstdio>

extern "C" {
#include "sstep.h"
}

/* ==============================================================================
 * HELPERS
 * =============================================================================*/

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr double kLsb = 1.0 / 32768.0; /* Q15 resolution */

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
constexpr bool kFixed = true;
double to_d(sstep_real_t x) { return static_cast<double>(x) / 32768.0; }
double acc_d(sstep_acc_t x) { return static_cast<double>(x) / 32768.0; }
sstep_real_t from_d(double x) { return sstep_sat(static_cast<int32_t>(std::lround(x * 32768.0))); }
sstep_acc_t acc_from_d(double x) { return static_cast<sstep_acc_t>(std::llround(x * 32768.0)); }
double gain_d(const sstep_gain& g) { return std::ldexp(g.mant / 32768.0, g.exp); }
#else
constexpr bool kFixed = false;
double to_d(sstep_real_t x) { return static_cast<double>(x); }
double acc_d(sstep_acc_t x) { return static_cast<double>(x); }
sstep_real_t from_d(double x) { return static_cast<sstep_real_t>(x); }
sstep_acc_t acc_from_d(double x) { return static_cast<sstep_acc_t>(x); }
double gain_d(const sstep_gain& g) { return static_cast<double>(g.value); }
#endif

/* Allowed error of one rounded operation: 1 LSB (fixed), float rounding (float) */
constexpr double kTol = kFixed ? (1.0 * kLsb) : 1e-6;

sstep_angle_t angle_from_rad(double rad)
{
    double turns = rad / (2.0 * kPi);
    turns -= std::floor(turns);
    return static_cast<sstep_angle_t>(static_cast<uint64_t>(std::llround(turns * 4294967296.0)) &
                                      0xFFFFFFFFULL);
}

} // namespace

/* ==============================================================================
 * CONSTANTS AND RANGES
 * =============================================================================*/

TEST(SstepMath, ConstantsSaturate)
{
    const sstep_real_t c[] = {SSTEP_REAL_C(0.5), SSTEP_REAL_C(-0.25), SSTEP_REAL_C(1.0),
                              SSTEP_REAL_C(2.0), SSTEP_REAL_C(-1.0), SSTEP_REAL_C(-3.0)};
    EXPECT_DOUBLE_EQ(to_d(c[0]), 0.5);
    EXPECT_DOUBLE_EQ(to_d(c[1]), -0.25);
    EXPECT_DOUBLE_EQ(to_d(c[2]), to_d(SSTEP_REAL_MAX));
    EXPECT_DOUBLE_EQ(to_d(c[3]), to_d(SSTEP_REAL_MAX));
    EXPECT_DOUBLE_EQ(to_d(c[4]), -1.0);
    EXPECT_DOUBLE_EQ(to_d(c[5]), -1.0);
    EXPECT_DOUBLE_EQ(to_d(SSTEP_REAL_MAX), 1.0 - kLsb); /* same range in both backends */
    EXPECT_NEAR(acc_d(SSTEP_ACC_MAX), 65536.0 - kLsb, kFixed ? 0.0 : (1.0 / 256.0));
    EXPECT_DOUBLE_EQ(acc_d(SSTEP_ACC_MIN), -65536.0);
}

/* ==============================================================================
 * BASIC OPERATIONS
 * =============================================================================*/

TEST(SstepMath, AddSubSaturate)
{
    EXPECT_NEAR(to_d(sstep_add(from_d(0.25), from_d(0.5))), 0.75, kTol);
    EXPECT_NEAR(to_d(sstep_sub(from_d(0.25), from_d(0.5))), -0.25, kTol);
    EXPECT_EQ(sstep_add(from_d(0.75), from_d(0.75)), SSTEP_REAL_MAX);
    EXPECT_EQ(sstep_sub(from_d(-0.75), from_d(0.75)), SSTEP_REAL_MIN);
    EXPECT_EQ(sstep_sub(SSTEP_REAL_MAX, SSTEP_REAL_MIN), SSTEP_REAL_MAX);
}

TEST(SstepMath, NegAbsSaturate)
{
    EXPECT_EQ(sstep_neg(SSTEP_REAL_MIN), SSTEP_REAL_MAX); /* -(-1.0) */
    EXPECT_EQ(sstep_abs(SSTEP_REAL_MIN), SSTEP_REAL_MAX);
    EXPECT_NEAR(to_d(sstep_neg(from_d(0.3))), -0.3, kTol);
    EXPECT_NEAR(to_d(sstep_abs(from_d(-0.3))), 0.3, kTol);
    EXPECT_NEAR(to_d(sstep_abs(from_d(0.3))), 0.3, kTol);
}

TEST(SstepMath, MulRoundsAndSaturates)
{
    EXPECT_EQ(sstep_mul(SSTEP_REAL_MIN, SSTEP_REAL_MIN), SSTEP_REAL_MAX); /* (-1)*(-1) */
    EXPECT_DOUBLE_EQ(to_d(sstep_mul(from_d(0.5), from_d(-0.5))), -0.25);
    double worst = 0.0;
    for (int i = -32768; i < 32768; i += 37)
    {
        for (int j = -32768; j < 32768; j += 1013)
        {
            sstep_real_t a = from_d(i * kLsb);
            sstep_real_t b = from_d(j * kLsb);
            double exact = to_d(a) * to_d(b);
            if (exact >= 1.0)
            {
                continue;
            }
            worst = std::fmax(worst, std::fabs(to_d(sstep_mul(a, b)) - exact));
        }
    }
    EXPECT_LE(worst, kFixed ? 0.5 * kLsb : 1e-7); /* rounded to nearest */
}

TEST(SstepMath, Clamp)
{
    const sstep_real_t lo = from_d(-0.5);
    const sstep_real_t hi = from_d(0.5);
    EXPECT_EQ(sstep_clamp(from_d(0.7), lo, hi), hi);
    EXPECT_EQ(sstep_clamp(from_d(-0.7), lo, hi), lo);
    EXPECT_EQ(sstep_clamp(from_d(0.1), lo, hi), from_d(0.1));
    EXPECT_EQ(sstep_acc_clamp(acc_from_d(3.0), acc_from_d(-2.0), acc_from_d(2.0)), acc_from_d(2.0));
    EXPECT_EQ(sstep_acc_clamp(acc_from_d(-3.0), acc_from_d(-2.0), acc_from_d(2.0)), acc_from_d(-2.0));
    EXPECT_EQ(sstep_acc_clamp(acc_from_d(1.0), acc_from_d(-2.0), acc_from_d(2.0)), acc_from_d(1.0));
}

TEST(SstepMath, AccumulatorSaturates)
{
    EXPECT_EQ(sstep_acc_add(SSTEP_ACC_MAX, acc_from_d(1.0)), SSTEP_ACC_MAX);
    EXPECT_EQ(sstep_acc_add(SSTEP_ACC_MIN, acc_from_d(-1.0)), SSTEP_ACC_MIN);
    EXPECT_NEAR(acc_d(sstep_acc_add(acc_from_d(100.5), acc_from_d(-0.25))), 100.25, kTol);
    EXPECT_NEAR(acc_d(sstep_acc_add(acc_from_d(1.0), acc_from_d(2.5))), 3.5, kTol);
    EXPECT_NEAR(acc_d(sstep_acc_add(acc_from_d(-1.0), acc_from_d(0.0))), -1.0, kTol);
    EXPECT_EQ(sstep_acc_to_real(acc_from_d(5.0)), SSTEP_REAL_MAX);
    EXPECT_EQ(sstep_acc_to_real(acc_from_d(-5.0)), SSTEP_REAL_MIN);
    EXPECT_EQ(sstep_acc_from_real(from_d(0.5)), acc_from_d(0.5));
}

/* ==============================================================================
 * GAINS
 * =============================================================================*/

TEST(SstepMath, GainFromRatioArguments)
{
    sstep_gain g{};
    EXPECT_EQ(sstep_gain_from_ratio(1, 0, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio(1, 1, nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio(32768, 1, &g), SSTEP_ERR_PARAM);  /* too large */
    EXPECT_EQ(sstep_gain_from_ratio(-65535, 2, &g), SSTEP_ERR_PARAM); /* rounds to 32768 */
    EXPECT_EQ(sstep_gain_from_ratio(65533, 2, &g), SSTEP_OK);         /* 32766.5 */
}

TEST(SstepMath, GainFromRatioAccuracy)
{
    const struct
    {
        int32_t num;
        int32_t den;
    } cases[] = {{1, 1},       {-1, 1},      {1, -1},     {-3, -7},     {32767, 1},
                 {1, 2047},    {-1, 2047},   {3000, 7},   {1400, 4000}, {INT32_MIN, 131072},
                 {7, 1000000}, {1, 30000000}, {0, 5},      {12345, 678}, {INT32_MAX, INT32_MIN}};
    for (const auto& c : cases)
    {
        sstep_gain g{};
        ASSERT_EQ(sstep_gain_from_ratio(c.num, c.den, &g), SSTEP_OK) << c.num << "/" << c.den;
        const double want = static_cast<double>(c.num) / static_cast<double>(c.den);
        /* relative error of a normalised Q15 mantissa: <= 2^-15; float: 2^-24 */
        double rel = 1.2e-7; /* float: 2^-23 */
        if (kFixed)
        {
            /* normalised mantissa: 2^-15; below 2^-16 the mantissa has fewer bits */
            rel = (std::fabs(want) < 1.0 / 65536.0) ? 1e-2 : 3.1e-5;
        }
        EXPECT_NEAR(gain_d(g), want, std::fabs(want) * rel + 1e-300) << c.num << "/" << c.den;
    }
}

TEST(SstepMath, GainFromRatioSmallest)
{
    /* the smallest non-zero int32 ratio is still representable (not rounded to 0) */
    sstep_gain g{};
    EXPECT_EQ(sstep_gain_from_ratio(1, INT32_MIN, &g), SSTEP_OK);
    EXPECT_NEAR(gain_d(g), -1.0 / 2147483648.0, 1e-12);
}

TEST(SstepMath, GainMul)
{
    sstep_gain g{};
    ASSERT_EQ(sstep_gain_from_ratio(25, 2, &g), SSTEP_OK); /* 12.5 */
    EXPECT_NEAR(acc_d(sstep_gain_mul(&g, from_d(0.5))), 6.25, 12.5 * 4e-5);
    EXPECT_NEAR(acc_d(sstep_gain_mul(&g, from_d(-1.0))), -12.5, 12.5 * 4e-5);

    ASSERT_EQ(sstep_gain_from_ratio(-1, 1000, &g), SSTEP_OK); /* small: exp < 0 */
    EXPECT_NEAR(acc_d(sstep_gain_mul(&g, from_d(0.5))), -0.0005, kTol);

    ASSERT_EQ(sstep_gain_from_ratio(32767, 1, &g), SSTEP_OK); /* exp = 15: no shift */
    EXPECT_NEAR(acc_d(sstep_gain_mul(&g, SSTEP_REAL_MAX)), 32767.0 * (1.0 - kLsb), 1.0);

    ASSERT_EQ(sstep_gain_from_ratio(0, 1, &g), SSTEP_OK);
    EXPECT_EQ(sstep_gain_mul(&g, SSTEP_REAL_MAX), acc_from_d(0.0));
}

TEST(SstepMath, GainMulInt)
{
    sstep_gain g{};
    /* ADC: 2047 counts per unit -> 1 count = 1/2047 pu */
    ASSERT_EQ(sstep_gain_from_ratio(1, 2047, &g), SSTEP_OK);
    EXPECT_NEAR(acc_d(sstep_gain_mul_int(&g, 2047)), 1.0, 2.0 * kLsb);
    EXPECT_NEAR(acc_d(sstep_gain_mul_int(&g, -1000)), -1000.0 / 2047.0, 2.0 * kLsb);
    /* inputs beyond 16 bits are saturated first (same in both backends) */
    EXPECT_EQ(sstep_gain_mul_int(&g, 100000), sstep_gain_mul_int(&g, 32767));
    EXPECT_EQ(sstep_gain_mul_int(&g, -100000), sstep_gain_mul_int(&g, -32768));

    /* exp >= 0: 3 counts per pu inverse -> gain 3 */
    ASSERT_EQ(sstep_gain_from_ratio(3, 1, &g), SSTEP_OK);
    EXPECT_NEAR(acc_d(sstep_gain_mul_int(&g, 1000)), 3000.0, 0.1);
    /* gain 30000 * 32767 > accumulator range: saturates */
    ASSERT_EQ(sstep_gain_from_ratio(30000, 1, &g), SSTEP_OK);
    EXPECT_EQ(sstep_gain_mul_int(&g, 32767), SSTEP_ACC_MAX);
    EXPECT_EQ(sstep_gain_mul_int(&g, -32768), SSTEP_ACC_MIN);
}

/* ==============================================================================
 * CONVERSIONS
 * =============================================================================*/

TEST(SstepMath, RealFromRatio)
{
    sstep_real_t x{};
    EXPECT_EQ(sstep_real_from_ratio(1, 0, &x), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_real_from_ratio(1, 1, nullptr), SSTEP_ERR_PARAM);
    ASSERT_EQ(sstep_real_from_ratio(1500, 4000, &x), SSTEP_OK);
    EXPECT_NEAR(to_d(x), 0.375, kTol);
    ASSERT_EQ(sstep_real_from_ratio(-1500, 4000, &x), SSTEP_OK);
    EXPECT_NEAR(to_d(x), -0.375, kTol);
    ASSERT_EQ(sstep_real_from_ratio(1500, -4000, &x), SSTEP_OK);
    EXPECT_NEAR(to_d(x), -0.375, kTol);
    ASSERT_EQ(sstep_real_from_ratio(5000, 4000, &x), SSTEP_OK);
    EXPECT_EQ(x, SSTEP_REAL_MAX);
    ASSERT_EQ(sstep_real_from_ratio(-5000, 4000, &x), SSTEP_OK);
    EXPECT_EQ(x, SSTEP_REAL_MIN);
    ASSERT_EQ(sstep_real_from_ratio(INT32_MIN, 1, &x), SSTEP_OK);
    EXPECT_EQ(x, SSTEP_REAL_MIN);
}

TEST(SstepMath, RealToUnits)
{
    EXPECT_EQ(sstep_real_to_units(acc_from_d(0.375), 4000), 1500);
    EXPECT_EQ(sstep_real_to_units(acc_from_d(-0.375), 4000), -1500);
    EXPECT_EQ(sstep_real_to_units(acc_from_d(0.5), -3), -2); /* -1.5 rounds away from zero */
    EXPECT_EQ(sstep_real_to_units(SSTEP_ACC_MAX, INT32_MAX), INT32_MAX);
    EXPECT_EQ(sstep_real_to_units(SSTEP_ACC_MIN, INT32_MAX), INT32_MIN);
    EXPECT_EQ(sstep_real_to_units(acc_from_d(0.0), 1000), 0);
}

/* ==============================================================================
 * SINE / COSINE
 * =============================================================================*/

TEST(SstepMath, SinCosExactPoints)
{
    const sstep_sincos q0 = sstep_sin_cos(0U);
    const sstep_sincos q1 = sstep_sin_cos(0x40000000UL);
    const sstep_sincos q2 = sstep_sin_cos(0x80000000UL);
    const sstep_sincos q3 = sstep_sin_cos(0xC0000000UL);
    EXPECT_NEAR(to_d(q0.sin), 0.0, kTol);
    EXPECT_NEAR(to_d(q0.cos), 1.0, kTol);
    EXPECT_NEAR(to_d(q1.sin), 1.0, kTol);
    EXPECT_NEAR(to_d(q1.cos), 0.0, kTol);
    EXPECT_NEAR(to_d(q2.sin), 0.0, kTol);
    EXPECT_NEAR(to_d(q2.cos), -1.0, kTol);
    EXPECT_NEAR(to_d(q3.sin), -1.0, kTol);
    EXPECT_NEAR(to_d(q3.cos), 0.0, kTol);
}

TEST(SstepMath, SinCosAccuracyFullTurn)
{
    double worst = 0.0;
    double sum_sq = 0.0;
    const uint32_t step = 4099U; /* odd step: visits every table interval and fraction */
    uint64_t count = 0U;
    for (uint64_t a = 0U; a < 0x100000000ULL; a += (uint64_t)step * 251U)
    {
        const sstep_angle_t ang = static_cast<sstep_angle_t>(a);
        const double rad = static_cast<double>(ang) * (2.0 * kPi / 4294967296.0);
        const sstep_sincos r = sstep_sin_cos(ang);
        const double es = std::fabs(to_d(r.sin) - std::sin(rad));
        const double ec = std::fabs(to_d(r.cos) - std::cos(rad));
        worst = std::fmax(worst, std::fmax(es, ec));
        sum_sq += es * es + ec * ec;
        count += 2U;
    }
    const double rms = std::sqrt(sum_sq / static_cast<double>(count));
    std::printf("[ measured ] sin/cos table bits=%u: max error %.3g (%.2f LSB), rms %.3g\n",
                static_cast<unsigned>(SSTEP_CFG_SINE_TABLE_BITS), worst, worst / kLsb, rms);
    /* interpolation error (pi/2/N)^2/8 + table and output rounding */
    const double h = kPi / 2.0 / static_cast<double>(1U << SSTEP_CFG_SINE_TABLE_BITS);
    EXPECT_LE(worst, h * h / 8.0 + (kFixed ? 1.5 * kLsb : 1e-6));
}

TEST(SstepMath, SinCosQuadrantShiftIsExact)
{
    for (uint32_t a = 0U; a < 0xC0000000UL; a += 0x00F0F0F1UL)
    {
        const sstep_sincos r0 = sstep_sin_cos(a);
        const sstep_sincos r1 = sstep_sin_cos(a + 0x40000000UL);
        EXPECT_EQ(r1.sin, r0.cos);
        EXPECT_EQ(r1.cos, sstep_neg(r0.sin));
    }
}

TEST(SstepMath, SinCosMagnitude)
{
    /* the current vector amplitude must not depend on the angle (torque ripple) */
    double lo = 2.0;
    double hi = 0.0;
    for (uint32_t a = 0U; a < 0x40000000UL; a += 0x0001F3A7UL)
    {
        const sstep_sincos r = sstep_sin_cos(a);
        const double m = std::hypot(to_d(r.sin), to_d(r.cos));
        lo = std::fmin(lo, m);
        hi = std::fmax(hi, m);
    }
    std::printf("[ measured ] |(sin, cos)| min %.6f max %.6f\n", lo, hi);
    EXPECT_GT(lo, 1.0 - 4.0 * kLsb);
    EXPECT_LT(hi, 1.0 + 2.0 * kLsb);
}

TEST(SstepMath, AngleHelperRoundTrip)
{
    EXPECT_EQ(angle_from_rad(0.0), 0U);
    EXPECT_EQ(angle_from_rad(kPi), 0x80000000UL);
}

/* ==============================================================================
 * 64-BIT RATIOS
 * =============================================================================*/

namespace
{

/* Reference: the largest s in 0..31 with round(n 2^s / d) <= 32767 (brute force) */
double ratio_reference(int64_t num, int64_t den, bool* ok)
{
    const bool neg = ((num < 0) != (den < 0));
    long double n = std::fabs(static_cast<long double>(num));
    long double d = std::fabs(static_cast<long double>(den));
    int best = -1;
    long double best_m = 0;
    for (int s = 0; s <= 31; s++)
    {
        long double m = std::floor(n * std::ldexp(1.0L, s) / d + 0.5L);
        if (m <= 32767.0L)
        {
            best = s;
            best_m = m;
        }
    }
    *ok = (best >= 0);
    return static_cast<double>((neg ? -best_m : best_m) * std::ldexp(1.0L, -best));
}

} // namespace

TEST(SstepMath, GainFromRatio64Arguments)
{
    sstep_gain g{};
    const int64_t lim = static_cast<int64_t>(1) << 62;
    EXPECT_EQ(sstep_gain_from_ratio64(1, 0, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(1, 1, nullptr), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(lim, 1, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(-lim, 1, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(1, lim, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(1, -lim, &g), SSTEP_ERR_PARAM);
    EXPECT_EQ(sstep_gain_from_ratio64(lim - 1, 1, &g), SSTEP_ERR_PARAM); /* too large */
    EXPECT_EQ(sstep_gain_from_ratio64(0, 7, &g), SSTEP_OK);
    EXPECT_DOUBLE_EQ(gain_d(g), 0.0);
}

TEST(SstepMath, GainFromRatio64MatchesBruteForce)
{
    /* the normalisation (estimate + one correction) must give the same result as
       trying every exponent; random operands over the whole range, both signs */
    uint64_t seed = 12345U;
    auto rnd = [&seed]() {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return seed;
    };
    int checked = 0;
    for (int k = 0; k < 200000; k++)
    {
        const int bn = static_cast<int>(rnd() % 62U);
        const int bd = static_cast<int>(rnd() % 62U);
        int64_t num = static_cast<int64_t>(rnd() >> (64 - bn - 1)) * ((rnd() & 1U) ? -1 : 1);
        int64_t den = static_cast<int64_t>(rnd() >> (64 - bd - 1)) * ((rnd() & 1U) ? -1 : 1);
        if (den == 0)
        {
            den = 1;
        }
        bool ok = false;
        const double want = ratio_reference(num, den, &ok);
        sstep_gain g{};
        const int r = sstep_gain_from_ratio64(num, den, &g);
        if (!ok)
        {
            EXPECT_EQ(r, SSTEP_ERR_PARAM) << num << "/" << den;
            continue;
        }
        ASSERT_EQ(r, SSTEP_OK) << num << "/" << den;
        checked++;
        if (kFixed)
        {
            /* identical up to the operand pre-shift of very large denominators (2^-46) */
            EXPECT_NEAR(gain_d(g), want, std::fabs(want) * 1e-9 + 1e-300) << num << "/" << den;
        }
        else
        {
            const double exact = static_cast<double>(num) / static_cast<double>(den);
            EXPECT_NEAR(gain_d(g), exact, std::fabs(exact) * 2.5e-7) << num << "/" << den;
        }
    }
    EXPECT_GT(checked, 100000);
}

TEST(SstepMath, GainFromRatio64EngineeringValues)
{
    /* Kp in pu for 17HS19 at 1 kHz: L wc I_base / V_base = 3 mH * 2 pi 1000 * 4 A / 24 V,
       2 pi = 710 / 113 (error 8.5e-8) */
    sstep_gain g{};
    ASSERT_EQ(sstep_gain_from_ratio64(3000LL * 1000 * 4000 * 710, 113LL * 24000 * 1000000, &g),
              SSTEP_OK);
    EXPECT_NEAR(gain_d(g), 3.0e-3 * 2.0 * kPi * 1000.0 * 4.0 / 24.0, 3.1416 * 4e-5);
}

/* ==============================================================================
 * ACCUMULATOR, INTEGRATOR AND POWER STAGE HELPERS
 * =============================================================================*/

TEST(SstepMath, AccAbsHalve)
{
    EXPECT_EQ(sstep_acc_abs(acc_from_d(-1.5)), acc_from_d(1.5));
    EXPECT_EQ(sstep_acc_abs(acc_from_d(1.5)), acc_from_d(1.5));
    EXPECT_EQ(sstep_acc_abs(SSTEP_ACC_MIN), SSTEP_ACC_MAX + (kFixed ? 0 : 0.00390625f));
    EXPECT_NEAR(acc_d(sstep_acc_halve(acc_from_d(3.0))), 1.5, kTol);
    EXPECT_NEAR(acc_d(sstep_acc_halve(acc_from_d(-3.0))), -1.5, kTol);
}

TEST(SstepMath, GainMulAcc)
{
    sstep_gain g{};
    ASSERT_EQ(sstep_gain_from_ratio(5, 2, &g), SSTEP_OK); /* 2.5 */
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(1.5))), 3.75, 2.5 * 4 * kLsb);
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(-3.0))), -7.5, 2.5 * 4 * kLsb);
    /* inputs beyond 4 pu are saturated */
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(100.0))), 10.0, 2.5 * 4 * kLsb);
    ASSERT_EQ(sstep_gain_from_ratio(1, 3, &g), SSTEP_OK); /* exp < 0 */
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(3.0))), 1.0, 4 * kLsb);
    ASSERT_EQ(sstep_gain_from_ratio(30000, 1, &g), SSTEP_OK); /* exp = 15: left shift */
    EXPECT_EQ(sstep_gain_mul_acc(&g, acc_from_d(3.0)), SSTEP_ACC_MAX);
    EXPECT_EQ(sstep_gain_mul_acc(&g, acc_from_d(-3.0)), SSTEP_ACC_MIN);
    ASSERT_EQ(sstep_gain_from_ratio(2, 1, &g), SSTEP_OK); /* exp = 2 */
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(1.25))), 2.5, 4 * kLsb);
    ASSERT_EQ(sstep_gain_from_ratio(13, 2, &g), SSTEP_OK); /* exp = 3: no shift (13 - exp = 10) */
    EXPECT_NEAR(acc_d(sstep_gain_mul_acc(&g, acc_from_d(0.5))), 3.25, 8 * kLsb);
}

TEST(SstepMath, IntegratorKeepsSmallIncrements)
{
    /* Ki * Ts = 0.0838 (17HS19, 1 kHz, 20 kHz): an error of one LSB adds 0.08 LSB per
       period; after 1000 periods the integrator must hold about 84 LSB, not 0 */
    sstep_gain ki{};
    ASSERT_EQ(sstep_gain_from_ratio(838, 10000, &ki), SSTEP_OK);
    sstep_integ_t integ{};
    const sstep_real_t e = kFixed ? static_cast<sstep_real_t>(1) : static_cast<sstep_real_t>(kLsb);
    for (int k = 0; k < 1000; k++)
    {
        integ = sstep_integ_add(integ, &ki, e);
    }
    EXPECT_NEAR(acc_d(sstep_integ_to_acc(integ)), 83.8 * kLsb, 1.0 * kLsb);
}

TEST(SstepMath, IntegratorSaturates)
{
    sstep_gain g{};
    ASSERT_EQ(sstep_gain_from_ratio(8, 1, &g), SSTEP_OK);
    sstep_integ_t integ{};
    for (int k = 0; k < 10; k++)
    {
        integ = sstep_integ_add(integ, &g, SSTEP_REAL_MAX);
    }
    EXPECT_NEAR(acc_d(sstep_integ_to_acc(integ)), 16.0, 1e-3); /* +16 pu limit */
    for (int k = 0; k < 20; k++)
    {
        integ = sstep_integ_add_acc(integ, &g, acc_from_d(-3.0));
    }
    EXPECT_NEAR(acc_d(sstep_integ_to_acc(integ)), -16.0, 1e-3);
    ASSERT_EQ(sstep_gain_from_ratio(1, 40, &g), SSTEP_OK); /* back-calculation size */
    sstep_integ_t i2{};
    i2 = sstep_integ_add_acc(i2, &g, acc_from_d(0.5));
    EXPECT_NEAR(acc_d(sstep_integ_to_acc(i2)), 0.0125, 4 * kLsb);
}

TEST(SstepMath, DivUnit)
{
    EXPECT_NEAR(to_d(sstep_div_unit(acc_from_d(1.0), acc_from_d(4.0))), 0.25, kTol);
    EXPECT_NEAR(to_d(sstep_div_unit(acc_from_d(1.0), acc_from_d(1.6))), 0.625, kTol);
    EXPECT_EQ(sstep_div_unit(acc_from_d(2.0), acc_from_d(2.0)), SSTEP_REAL_MAX); /* 1.0 */
    EXPECT_NEAR(to_d(sstep_div_unit(acc_from_d(0.3), acc_from_d(0.9))), 1.0 / 3.0, kTol);
    EXPECT_EQ(sstep_div_unit(acc_from_d(0.0), acc_from_d(0.9)), from_d(0.0));
    EXPECT_NEAR(to_d(sstep_div_unit(acc_from_d(1000.0), acc_from_d(3000.0))), 1.0 / 3.0, kTol);
}

TEST(SstepMath, AccScale)
{
    EXPECT_NEAR(acc_d(sstep_acc_scale(acc_from_d(1.2), from_d(0.5))), 0.6, kTol);
    EXPECT_NEAR(acc_d(sstep_acc_scale(acc_from_d(-1.2), from_d(0.5))), -0.6, kTol);
    EXPECT_NEAR(acc_d(sstep_acc_scale(acc_from_d(5.0), from_d(0.5))), 1.0, kLsb); /* x < 2 */
}

TEST(SstepMath, DutyToTicks)
{
    EXPECT_EQ(sstep_acc_to_ticks(acc_from_d(0.5), 4250U), 2125U);
    EXPECT_EQ(sstep_acc_to_ticks(acc_from_d(1.0), 4250U), 4250U);
    EXPECT_EQ(sstep_acc_to_ticks(acc_from_d(1.5), 4250U), 4250U);
    EXPECT_EQ(sstep_acc_to_ticks(acc_from_d(-0.1), 4250U), 0U);
    EXPECT_EQ(sstep_acc_to_ticks(acc_from_d(0.25), 65535U), 16384U);
}

#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
TEST(SstepMath, ShiftRound)
{
    EXPECT_EQ(sstep_shift_round(12345, 0), 12345);
    EXPECT_EQ(sstep_shift_round(5, 1), 3);    /* 2.5 rounds up */
    EXPECT_EQ(sstep_shift_round(-5, 1), -2);  /* -2.5 rounds up */
    EXPECT_EQ(sstep_shift_round(3, -2), 12);
    EXPECT_EQ(sstep_shift_round(INT32_MAX / 2, -2), INT32_MAX);
    EXPECT_EQ(sstep_shift_round(INT32_MIN / 2, -2), INT32_MIN);
}
#endif
