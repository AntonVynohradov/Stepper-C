/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Test helpers: configuration of 17HS19-2004S1 on B-G431B-ESC1, fake hardware, motor model.
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


#ifndef SSTEP_TEST_PLANT_HPP
#define SSTEP_TEST_PLANT_HPP

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <cmath>
#include <cstdint>
#include <vector>

extern "C" {
#include "sstep.h"
#include "sstep_internal.h"
}

namespace sstep_test
{

/* ==============================================================================
 * MOTOR (same equations and parameters as sfs-stepper-sim)
 * =============================================================================*/

struct Motor
{
    double R = 1.60;      /* Ohm */
    double L = 3.0e-3;    /* H */
    double Kt = 0.2086;   /* N*m/A */
    double Ns = 200.0;    /* full steps per revolution */
    double J = 82e-7;     /* kg*m^2 */
    double B = 1e-5;      /* N*m*s/rad */
};

struct State
{
    double iA = 0.0;
    double iB = 0.0;
    double w = 0.0;
    double th = 0.0;
};

inline State deriv(const Motor& m, const State& x, double vA, double vB, double TL, bool locked)
{
    const double p = m.Ns / 4.0;
    const double s = std::sin(p * x.th);
    const double c = std::cos(p * x.th);
    const double Te = m.Kt * (-x.iA * s + x.iB * c);
    State d;
    d.iA = (vA - m.R * x.iA + m.Kt * x.w * s) / m.L;
    d.iB = (vB - m.R * x.iB - m.Kt * x.w * c) / m.L;
    d.w = locked ? 0.0 : (Te - TL - m.B * x.w) / m.J;
    d.th = locked ? 0.0 : x.w;
    return d;
}

inline State axpy(const State& x, double h, const State& k)
{
    return State{x.iA + h * k.iA, x.iB + h * k.iB, x.w + h * k.w, x.th + h * k.th};
}

inline State rk4(const Motor& m, const State& x, double vA, double vB, double TL, bool locked,
                 double h)
{
    const State k1 = deriv(m, x, vA, vB, TL, locked);
    const State k2 = deriv(m, axpy(x, h / 2, k1), vA, vB, TL, locked);
    const State k3 = deriv(m, axpy(x, h / 2, k2), vA, vB, TL, locked);
    const State k4 = deriv(m, axpy(x, h, k3), vA, vB, TL, locked);
    State r;
    r.iA = x.iA + h / 6 * (k1.iA + 2 * k2.iA + 2 * k3.iA + k4.iA);
    r.iB = x.iB + h / 6 * (k1.iB + 2 * k2.iB + 2 * k3.iB + k4.iB);
    r.w = x.w + h / 6 * (k1.w + 2 * k2.w + 2 * k3.w + k4.w);
    r.th = x.th + h / 6 * (k1.th + 2 * k2.th + 2 * k3.th + k4.th);
    return r;
}

/* ==============================================================================
 * FAKE HARDWARE
 * =============================================================================*/

struct Hal
{
    uint16_t compare[SSTEP_LEGS_MAX] = {0, 0, 0, 0};
    bool enabled = false;
    int pwm_result = 0;
    int enable_result = 0;
    int pwm_writes = 0;
    uint16_t faults = 0;
    int fault_calls = 0;
    std::vector<sstep_event> events;
};

inline int hal_pwm(void* ctx, const uint16_t cmp[SSTEP_LEGS_MAX])
{
    Hal* h = static_cast<Hal*>(ctx);
    for (unsigned k = 0; k < SSTEP_LEGS_MAX; k++)
    {
        h->compare[k] = cmp[k];
    }
    h->pwm_writes++;
    return h->pwm_result;
}

inline int hal_enable(void* ctx, bool en)
{
    Hal* h = static_cast<Hal*>(ctx);
    h->enabled = en && (h->enable_result >= 0);
    return h->enable_result;
}

inline void hal_fault(void* ctx, uint16_t f)
{
    Hal* h = static_cast<Hal*>(ctx);
    h->faults = static_cast<uint16_t>(h->faults | f);
    h->fault_calls++;
}

inline void hal_event(void* ctx, sstep_event e)
{
    static_cast<Hal*>(ctx)->events.push_back(e);
}

/* ==============================================================================
 * CONFIGURATION: 17HS19-2004S1, B-G431B-ESC1 style stage, 24 V, 20 kHz / 1 kHz
 * =============================================================================*/

constexpr double kIFull = 4.0;       /* A at ADC full scale */
constexpr int kSpan = 2047;          /* counts from 0 A to +4 A */
constexpr int kZero = 2048;          /* counts at 0 A */
constexpr double kVFull = 34.320;    /* V at ADC 4095 */

inline sstep_config make_config(Hal* hal, sstep_topology topo = SSTEP_TOPOLOGY_SHARED_LEG)
{
    sstep_config c{};
    c.motor.r_mohm = 1600U;
    c.motor.l_uh = 3000U;
    c.motor.i_rated_ma = 2000U;
    c.motor.ke_uv_s_per_rad = 208600U;
    c.motor.full_steps_per_rev = 200U;
    c.stage.topology = topo;
    c.stage.vbus_nominal_mv = 24000U;
    c.stage.vbus_min_mv = 10000U;
    c.stage.vbus_max_mv = 30000U;
    c.stage.i_full_scale_ma = 4000U;
    c.stage.i_trip_ma = 3500U;
    c.stage.pwm_period_ticks = 4250U;
    c.stage.pwm_freq_hz = 20000U;
    c.stage.dead_time_ns = 0U;
    c.stage.adc_i_zero = kZero;
    c.stage.adc_i_span = kSpan;
    c.stage.adc_vbus_mv_full = 34320U;
    c.stage.adc_full = 4095U;
    c.ctrl.drive_mode = SSTEP_DRIVE_CURRENT;
    c.ctrl.fast_loop_hz = 20000U;
    c.ctrl.slow_loop_hz = 1000U;
    c.ctrl.microsteps = 16U;
    c.ctrl.current_bw_hz = 1000U;
    c.ctrl.i_run_ma = 2000U;
    c.ctrl.i_hold_ma = 1000U;
    c.ctrl.hold_delay_ms = 100U;
    c.ctrl.v_max = 16000U;
    c.ctrl.accel = 160000U;
    c.ctrl.decel = 0U;
    c.hal.context = hal;
    c.hal.pwm_write = hal_pwm;
    c.hal.output_enable = hal_enable;
    c.hal.on_fault = hal_fault;
    c.hal.on_event = hal_event;
    return c;
}

/* ==============================================================================
 * SOFTWARE IN THE LOOP: library + motor, one period computation delay
 * =============================================================================*/

struct Record
{
    double t, iA, iB, w, th, vA, vB;
};

struct Sim
{
    Motor mot;
    Hal hal;
    sstep drv{};
    sstep_config cfg{};
    State x;
    double vbus = 24.0;
    double TL = 0.0;
    bool locked = false;
    int nsub = 4;
    uint16_t cmp_prev[SSTEP_LEGS_MAX] = {0, 0, 0, 0};
    uint64_t tick = 0;
    std::vector<Record> rec;
    bool record = true;
    int64_t phi_turns = 0;      /* unwrapped electrical angle applied [2^-32 turn] */
    uint32_t phi_last = 0U;

    explicit Sim(sstep_topology topo = SSTEP_TOPOLOGY_SHARED_LEG) { cfg = make_config(&hal, topo); }

    int init() { return sstep_init(&drv, &cfg); }

    /* Private driver state, for white-box checks */
    sstep_ctx& ctx() { return *sstep_ctx_of(&drv); }

    static uint16_t adc_current(double i)
    {
        long v = std::lround(kZero + i / kIFull * kSpan);
        return static_cast<uint16_t>(v < 0 ? 0 : (v > 4095 ? 4095 : v));
    }

    sstep_sample sample() const
    {
        sstep_sample s{};
        s.i_a = adc_current(x.iA);
        s.i_b = adc_current(x.iB);
        s.vbus = static_cast<uint16_t>(std::lround(vbus / kVFull * 4095.0));
        return s;
    }

    /* phase voltages from the leg compare values of the previous period */
    void phase_voltages(double* vA, double* vB) const
    {
        const double per = cfg.stage.pwm_period_ticks;
        double leg[SSTEP_LEGS_MAX];
        for (unsigned k = 0; k < SSTEP_LEGS_MAX; k++)
        {
            leg[k] = cmp_prev[k] / per * vbus;
        }
        if (!hal.enabled)
        {
            *vA = 0.0;
            *vB = 0.0;
        }
        else if (cfg.stage.topology == SSTEP_TOPOLOGY_FULL_BRIDGE)
        {
            *vA = leg[0] - leg[1];
            *vB = leg[2] - leg[3];
        }
        else
        {
            *vA = leg[0] - leg[2];
            *vB = leg[1] - leg[2];
        }
    }

    void step()
    {
        const double Ts = 1.0 / cfg.ctrl.fast_loop_hz;
        const uint32_t ratio = cfg.ctrl.fast_loop_hz / cfg.ctrl.slow_loop_hz;
        if ((tick % ratio) == 0U)
        {
            (void)sstep_slow_loop(&drv);
        }
        const sstep_sample s = sample();
        (void)sstep_fast_loop(&drv, &s);
        /* angle used in this period: the fast loop advances it after use */
        const uint32_t used = ctx().cur.angle - static_cast<uint32_t>(ctx().cur.dangle);
        phi_turns += static_cast<int32_t>(used - phi_last);
        phi_last = used;
        double vA = 0.0;
        double vB = 0.0;
        phase_voltages(&vA, &vB);
        if (record)
        {
            rec.push_back(Record{tick * Ts, x.iA, x.iB, x.w, x.th, vA, vB});
        }
        for (int k = 0; k < nsub; k++)
        {
            x = rk4(mot, x, vA, vB, TL, locked, Ts / nsub);
        }
        for (unsigned k = 0; k < SSTEP_LEGS_MAX; k++)
        {
            cmp_prev[k] = hal.compare[k];
        }
        tick++;
    }

    void run_for(double seconds)
    {
        const auto n = static_cast<uint64_t>(std::llround(seconds * cfg.ctrl.fast_loop_hz));
        for (uint64_t k = 0; k < n; k++)
        {
            step();
        }
    }

    /* electrical angle applied minus rotor angle [deg]: positive = rotor behind */
    double load_angle_deg() const
    {
        const double phi = static_cast<double>(phi_turns) / 4294967296.0 * 360.0;
        return phi - mot.Ns / 4.0 * x.th * 180.0 / 3.14159265358979323846;
    }

    double rotor_usteps() const
    {
        return mot.Ns / 4.0 * x.th / (3.14159265358979323846 / 2.0) * cfg.ctrl.microsteps;
    }
};

} // namespace sstep_test

#endif /* SSTEP_TEST_PLANT_HPP */
