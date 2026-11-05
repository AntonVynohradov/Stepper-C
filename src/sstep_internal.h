/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Private interface between the library modules (power stage, motion, control).
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


#ifndef SSTEP_INTERNAL_H
#define SSTEP_INTERNAL_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stdint.h>

#include "sstep_ctrl.h"
#include "sstep_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** @brief Motion generator modes. */
#define SSTEP_MODE_HOLD 0U
#define SSTEP_MODE_MOVE 1U
#define SSTEP_MODE_RUN 2U

/** @brief Result flags of sstep_motion_step(). */
#define SSTEP_MOTION_DONE 0x01U

/** @brief Fixed point position: 1 microstep = 2^16. */
#define SSTEP_POS_ONE ((int64_t)65536)

/** @brief A move ends when it is closer than this to the target and slow enough
 *         (1/1024 microstep). */
#define SSTEP_POS_SNAP ((int64_t)64)

/** @brief Value of sstep_ctx::ready after a successful sstep_init() ("STPC"). A zeroed or
 *         never initialized instance holds another value: every function but sstep_init()
 *         then returns ::SSTEP_ERR_STATE instead of using an empty configuration. */
#define SSTEP_READY 0x53545043U

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

/** @brief Result of the power stage mapping. */
struct sstep_bridge_out
{
    uint16_t compare[SSTEP_LEGS_MAX]; /**< Compare values per leg. */
    sstep_acc_t v[2];                 /**< Applied phase voltages [pu of V_base]. */
    bool limited;                     /**< The request was scaled down. */
};

/* ==============================================================================
 * DRIVER INSTANCE
 * =============================================================================*/

/** @brief Setpoint handed from the slow loop to the fast loop. */
struct sstep_setpoint
{
    sstep_angle_t angle;        /**< Electrical angle at the start of the slow period. */
    int32_t dangle;             /**< Angle increment per fast loop period. */
    sstep_real_t i_amp;         /**< Current amplitude [pu]. */
    sstep_real_t ff;            /**< Back-EMF feed-forward amplitude [pu of V_base]. */
    struct sstep_gain inv_vbus; /**< V_base / measured bus voltage. */
};

/**
 * @brief Motion generator state. Integer arithmetic in both numeric backends:
 *        positions in 1/65536 microstep, velocities in 1/65536 microstep per slow loop
 *        period, accelerations in 1/65536 microstep per period squared.
 */
struct sstep_motion
{
    int64_t pos;        /**< Position [Q16 microsteps]. */
    int64_t target;     /**< Move target [Q16 microsteps]. */
    int64_t brake_vmax; /**< Braking distance from v_max [Q16 microsteps]. */
    int32_t vel;        /**< Velocity [Q16 microsteps / period]. */
    int32_t v_run;      /**< Requested velocity of a RUN command. */
    int32_t v_max;      /**< Velocity limit. */
    int32_t acc;        /**< Acceleration limit [Q16 microsteps / period^2]. */
    int32_t dec;        /**< Deceleration limit. */
    uint8_t mode;       /**< 0 hold, 1 move, 2 run. */
};

/** @brief Driver state behind the opaque ::sstep. Its layout may change in any release. */
struct sstep_ctx
{
    struct sstep_config cfg; /**< Copy of the configuration. */

    /* Constants derived by sstep_init() */
    struct sstep_gain kp;      /**< Proportional gain [pu V / pu A]. */
    struct sstep_gain ki;      /**< Integral gain per fast loop period. */
    struct sstep_gain kb;      /**< Anti-windup gain Ts / Tt. */
    struct sstep_gain r_pu;    /**< Phase resistance [pu] (voltage drive). */
    struct sstep_gain adc_i;   /**< Current per ADC count [pu]. */
    struct sstep_gain adc_v;   /**< Bus voltage per ADC count [pu]. */
    struct sstep_gain ff_gain; /**< Feed-forward per microstep/s [pu]. */
    sstep_real_t i_run;        /**< Run current [pu]. */
    sstep_real_t i_hold;       /**< Hold current [pu]. */
    uint16_t i_trip_counts;    /**< Overcurrent trip [ADC counts from zero]. */
    uint16_t vbus_min_raw;     /**< Undervoltage trip [ADC counts], 0 = off. */
    uint16_t vbus_max_raw;     /**< Overvoltage trip [ADC counts], 0 = off. */
    uint16_t ratio;            /**< Fast loop periods per slow loop period. */
    uint32_t hold_ticks;       /**< Slow periods at standstill before hold. */
    uint8_t angle_shift;       /**< 14 - log2(microsteps). */

    /* Stall detection constants, sstep_stall_*() */
    struct sstep_gain emf_a;     /**< L / Ts + R / 2 [pu V / pu A]. */
    struct sstep_gain emf_b;     /**< L / Ts - R / 2 [pu V / pu A]. */
    struct sstep_gain emf_alpha; /**< Ts / tau of the back-EMF filter. */
    uint32_t stall_ticks;        /**< Slow periods of stall to trip; 0 = detection off. */
    int32_t stall_emf_mv;        /**< Back-EMF of stall_speed [mV]. */

    /* Slow loop -> fast loop hand-off (double buffer) */
    struct sstep_setpoint sp[2]; /**< Setpoints; the fast loop owns sp[sp_active]. */
    volatile uint8_t sp_active;  /**< Buffer in use by the fast loop. */
    volatile uint8_t sp_pending; /**< 1: sp[1 - sp_active] is complete and new. */

    /* Fast loop state */
    struct sstep_setpoint cur;        /**< Setpoint in use. */
    uint16_t adv;                     /**< Angle increments applied since the hand-off. */
    sstep_integ_t integ[2];           /**< PI integrators [pu V]. */
    sstep_real_t i_meas[2];           /**< Last measured phase currents [pu]. */
    volatile uint16_t vbus_raw;       /**< Last bus voltage [ADC counts]. */
    uint16_t compare[SSTEP_LEGS_MAX]; /**< Last compare values. */
    sstep_real_t i_prev[2];           /**< Phase currents of the previous period [pu]. */
    sstep_acc_t v_hist[2][2];         /**< Applied phase voltages computed 1 and 2 periods
                                           ago [pu V]. */
    sstep_integ_t emf[2];             /**< Filtered back-EMF, d and q axis [pu V]. */

    /* Slow loop state */
    struct sstep_motion mo;     /**< Motion generator. */
    sstep_angle_t angle_offset; /**< Electrical angle of position 0. */
    uint32_t still;             /**< Slow periods at standstill. */
    bool hold_active;           /**< Hold current applied. */
    struct sstep_gain inv_vbus; /**< Last V_base / bus voltage. */
    int32_t vel_cmd;            /**< Commanded velocity of the period in use [microsteps/s]. */
    sstep_real_t ff_cmd;        /**< Back-EMF of vel_cmd [pu V]. */
    int64_t emf_off[2];         /**< Mean back-EMF at rest, d and q [mV * 2^16]. */
    uint32_t stall_count;       /**< Stall detector up/down counter. */
    uint32_t stall_hold;        /**< Slow periods the condition stays true after a reversal. */

    volatile uint16_t faults;        /**< Active faults. */
    volatile enum sstep_state state; /**< Driver state. */
    uint32_t ready;                  /**< ::SSTEP_READY after a successful sstep_init(). */
};

/**
 * @brief  Driver state of an instance.
 * @note   MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md): ::sstep is the
 *         opaque storage of ::sstep_ctx; size and alignment are checked in sstep_ctrl.c.
 * @param  drv  Driver instance, may be NULL.
 * @return Driver state, NULL if drv is NULL.
 */
static inline struct sstep_ctx* sstep_ctx_of(struct sstep* drv)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct sstep_ctx*)drv;
}

/** @brief Read-only variant of sstep_ctx_of(). */
static inline const struct sstep_ctx* sstep_ctx_of_const(const struct sstep* drv)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct sstep_ctx*)drv;
}

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS (library internal)
 * =============================================================================*/

/**
 * @brief  Map requested phase voltages to leg compare values (averaged power stage).
 *         The voltage vector is scaled down with its direction kept when the stage cannot
 *         produce it: full bridge |a|, |b| <= 1, shared leg max(a,b,0) - min(a,b,0) <= 1,
 *         a and b relative to the measured bus voltage. Shared leg: common leg centred.
 * @param  topology  Power stage topology (compiled in).
 * @param  va, vb    Requested phase voltages [pu of V_base].
 * @param  inv_vbus  V_base / bus voltage.
 * @param  vbus      Bus voltage [pu of V_base], 0 .. 2.
 * @param  period    Compare value for 100 % duty cycle.
 * @param  out       Result.
 */
void sstep_bridge_map(enum sstep_topology topology, sstep_acc_t va, sstep_acc_t vb,
                      const struct sstep_gain* inv_vbus, sstep_acc_t vbus, uint16_t period,
                      struct sstep_bridge_out* out);

/**
 * @brief  One slow loop period of the motion generator: trapezoidal velocity profile with
 *         one-period look-ahead braking for moves, exact stop on the target.
 * @param  mo  Motion state; pos and vel are updated.
 * @return ::SSTEP_MOTION_DONE when a move reached its target in this period, else 0.
 */
uint8_t sstep_motion_step(struct sstep_motion* mo);

/**
 * @brief  Integer square root: floor(sqrt(x)).
 * @param  x  Value.
 * @return floor(sqrt(x)).
 */
uint32_t sstep_isqrt64(uint64_t x);

/**
 * @brief  Reset the stall detector when the outputs are enabled: empty voltage history,
 *         filters at zero, current history from the last measurement.
 * @param  m  Driver state.
 */
void sstep_stall_reset(struct sstep_ctx* m);

/**
 * @brief  Back-EMF observer of one fast loop period (fast loop, current drive). Uses the
 *         voltage computed two periods ago (applied during the period that just ended),
 *         the measured currents and the commanded angle, then stores v_applied.
 * @param  m          Driver state; i_meas holds the currents of this period.
 * @param  sc         Sine and cosine of the commanded angle of this period.
 * @param  v_applied  Phase voltages computed in this period [pu V].
 */
void sstep_stall_observe(struct sstep_ctx* m, const struct sstep_sincos* sc,
                         const sstep_acc_t v_applied[2]);

/**
 * @brief  Stall decision of one slow loop period, against the command of the period that
 *         just ended (vel_cmd, ff_cmd).
 * @param  m  Driver state.
 * @return true when the stall condition has persisted for stall_time_ms.
 */
bool sstep_stall_check(struct sstep_ctx* m);

/**
 * @brief  Fill the observer fields of a status (zero when stall detection is off).
 * @param  m       Driver state.
 * @param  status  Status to complete.
 */
void sstep_stall_status(const struct sstep_ctx* m, struct sstep_status* status);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_INTERNAL_H */
