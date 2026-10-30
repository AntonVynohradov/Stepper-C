/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public API: configuration, hardware callbacks, control loops, motion commands.
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


#ifndef SSTEP_CTRL_H
#define SSTEP_CTRL_H

/**
 * @defgroup sstep_ctrl Control API
 * @brief    Initialise a driver, run its loops and command motion.
 *
 * **Execution model.** One driver instance (`struct sstep`) controls one motor. The
 * application calls two functions periodically:
 *
 * | Function | Rate | Context | Work |
 * | --- | --- | --- | --- |
 * | sstep_fast_loop() | `fast_loop_hz` (PWM rate or an integer fraction of it) | PWM / ADC interrupt | current control, voltage limit, topology mapping, PWM write |
 * | sstep_slow_loop() | `slow_loop_hz` (typically 1 kHz) | main loop or a timer task | trajectory, microstep angle, hold current, fault handling |
 *
 * All other functions are called from the same context as sstep_slow_loop(). The library
 * never reads a clock, never blocks, allocates no memory and has no global state: time
 * is given by the configured loop rates.
 *
 * **Hardware access** is done only through the callbacks in ::sstep_callbacks, given once
 * to sstep_init(). Measurements are passed to sstep_fast_loop() as raw ADC counts.
 * @{
 */

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sstep_config.h"
#include "sstep_defs.h"
#include "sstep_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * CALLBACK TYPES
 * =============================================================================*/

/**
 * @brief  Write the PWM compare values of all legs. Called from sstep_fast_loop().
 * @param  context  ::sstep_callbacks::context.
 * @param  compare  One compare value per leg, 0 .. `pwm_period_ticks`
 *                  (0 = low-side on, `pwm_period_ticks` = high-side on).
 *                  Entries above the leg count of the topology are 0.
 *                  Valid only during the call.
 * @retval 0        Success.
 * @retval <0       Error: the driver switches the outputs off with ::SSTEP_FAULT_HAL.
 */
typedef int (*sstep_pwm_write_fn)(void* context, const uint16_t compare[SSTEP_LEGS_MAX]);

/**
 * @brief  Enable or disable the power stage outputs (disabled = all legs high impedance).
 * @param  context  ::sstep_callbacks::context.
 * @param  enable   true to enable the gate drivers, false to switch every leg off.
 * @retval 0        Success.
 * @retval <0       Error, reported as ::SSTEP_FAULT_HAL.
 */
typedef int (*sstep_output_enable_fn)(void* context, bool enable);

/**
 * @brief  Notify the application of new faults (optional). Called from the context that
 *         detected the fault, which may be sstep_fast_loop() (interrupt).
 * @param  context  ::sstep_callbacks::context.
 * @param  faults   Newly raised ::SSTEP_FAULT_OVERCURRENT ... flags.
 */
typedef void (*sstep_fault_fn)(void* context, uint16_t faults);

/** @brief Events reported through ::sstep_event_fn. */
enum sstep_event
{
    SSTEP_EVENT_MOVE_DONE = 0,   /**< Target position reached, velocity zero. */
    SSTEP_EVENT_HOLD_CURRENT = 1 /**< Current reduced to the hold current after standstill. */
};

/**
 * @brief  Notify the application of a motion event (optional). Called from
 *         sstep_slow_loop().
 * @param  context  ::sstep_callbacks::context.
 * @param  event    The event.
 */
typedef void (*sstep_event_fn)(void* context, enum sstep_event event);

/* ==============================================================================
 * CONFIGURATION TYPES
 * =============================================================================*/

/** @brief Hardware callbacks. `pwm_write` and `output_enable` are required. */
struct sstep_callbacks
{
    void* context;                        /**< Passed unchanged to every callback. */
    sstep_pwm_write_fn pwm_write;         /**< Required. */
    sstep_output_enable_fn output_enable; /**< Required. */
    sstep_fault_fn on_fault;              /**< Optional, NULL if unused. */
    sstep_event_fn on_event;              /**< Optional, NULL if unused. */
};

/** @brief Motor parameters, from the datasheet or measured. Example: 17HS19-2004S1. */
struct sstep_motor_cfg
{
    uint32_t r_mohm;             /**< Phase resistance at 25 degC [mOhm]. 17HS19: 1600. */
    uint32_t l_uh;               /**< Phase inductance [uH]. 17HS19: 3000. */
    uint32_t i_rated_ma;         /**< Rated phase current [mA]. 17HS19: 2000. */
    uint32_t ke_uv_s_per_rad;    /**< Back-EMF constant, peak phase voltage per mechanical
                                      speed [uV*s/rad] (equals Kt in uN*m/A).
                                      0 = unknown: no back-EMF feed-forward. */
    uint16_t full_steps_per_rev; /**< Full steps per revolution. 1.8 deg: 200. */
};

/** @brief Power stage and measurement parameters. Example: B-G431B-ESC1. */
struct sstep_stage_cfg
{
    enum sstep_topology topology; /**< Power stage topology. ESC1: ::SSTEP_TOPOLOGY_SHARED_LEG. */
    uint32_t vbus_nominal_mv;     /**< Nominal bus voltage [mV]; base of the per-unit voltage. */
    uint32_t vbus_min_mv;         /**< Undervoltage trip [mV]; 0 = disabled. */
    uint32_t vbus_max_mv;         /**< Overvoltage trip [mV]; 0 = disabled. */
    uint32_t i_full_scale_ma;     /**< Phase current at ADC full scale [mA]; base of the
                                       per-unit current. */
    uint32_t i_trip_ma;           /**< Overcurrent trip [mA]; must not exceed i_full_scale_ma. */
    uint16_t pwm_period_ticks;    /**< Compare value for 100 % duty cycle. */
    uint32_t pwm_freq_hz;         /**< PWM frequency [Hz]. */
    uint32_t dead_time_ns;        /**< Dead time [ns], for compensation; 0 = none. */
    uint16_t adc_i_zero;          /**< Current ADC reading at 0 A [counts]. */
    int32_t adc_i_span;           /**< Counts from 0 A to +i_full_scale_ma; negative for an
                                       inverting amplifier. */
    uint32_t adc_vbus_mv_full;    /**< Bus voltage at ADC reading adc_full [mV]. */
    uint16_t adc_full;            /**< Largest ADC reading (4095 for 12 bits). */
};

/** @brief How the phase voltages are obtained. */
enum sstep_drive_mode
{
    /** Closed-loop current control (PI per phase, back-EMF feed-forward). Requires phase
     *  current measurement. */
    SSTEP_DRIVE_CURRENT = 0,
    /** Open-loop voltage control: V = R * I_ref (+ back-EMF feed-forward). For boards
     *  without current measurement; torque drops at speed. */
    SSTEP_DRIVE_VOLTAGE = 1
};

/** @brief Control and motion parameters. All of them except the loop rates can be changed
 *         at run time with the sstep_set_*() functions. */
struct sstep_ctrl_cfg
{
    enum sstep_drive_mode drive_mode; /**< Current or voltage drive. */
    uint32_t fast_loop_hz;            /**< Rate of sstep_fast_loop() [Hz]. */
    uint32_t slow_loop_hz;            /**< Rate of sstep_slow_loop() [Hz]. */
    uint16_t microsteps;              /**< Microsteps per full step (power of two,
                                           1 .. ::SSTEP_CFG_MICROSTEPS_MAX). */
    uint32_t current_bw_hz;           /**< Current loop bandwidth [Hz]; the PI gains are
                                           computed from R, L and this value. */
    uint32_t i_run_ma;                /**< Peak phase current while moving [mA]. */
    uint32_t i_hold_ma;               /**< Peak phase current at standstill [mA]. */
    uint32_t hold_delay_ms;           /**< Standstill time before switching to i_hold_ma [ms]. */
    uint32_t v_max;                   /**< Maximum velocity [microsteps/s]. */
    uint32_t accel;                   /**< Acceleration [microsteps/s^2]. */
    uint32_t decel;                   /**< Deceleration [microsteps/s^2]; 0 = same as accel. */
};

/** @brief Complete configuration given to sstep_init(). Copied by sstep_init(); the
 *         structure may be discarded afterwards. */
struct sstep_config
{
    struct sstep_motor_cfg motor;   /**< Motor. */
    struct sstep_stage_cfg stage;   /**< Power stage and measurement. */
    struct sstep_ctrl_cfg ctrl;     /**< Control and motion. */
    struct sstep_callbacks hal;     /**< Hardware callbacks. */
};

/* ==============================================================================
 * MEASUREMENT AND STATUS TYPES
 * =============================================================================*/

/** @brief Raw measurements of one fast loop period, as read from the ADC. */
struct sstep_sample
{
    uint16_t i_a; /**< Phase A current [ADC counts]. Ignored in ::SSTEP_DRIVE_VOLTAGE. */
    uint16_t i_b; /**< Phase B current [ADC counts]. Ignored in ::SSTEP_DRIVE_VOLTAGE. */
    uint16_t vbus; /**< Bus voltage [ADC counts]. */
};

/** @brief Snapshot returned by sstep_get_status(), in engineering units. */
struct sstep_status
{
    enum sstep_state state; /**< Driver state. */
    uint16_t faults;        /**< Active ::SSTEP_FAULT_OVERCURRENT ... flags. */
    int32_t position;       /**< Commanded position [microsteps]. */
    int32_t target;         /**< Target of the current move [microsteps]. */
    int32_t velocity;       /**< Commanded velocity [microsteps/s]. */
    int32_t i_a_ma;         /**< Last measured phase A current [mA]. */
    int32_t i_b_ma;         /**< Last measured phase B current [mA]. */
    uint32_t vbus_mv;       /**< Last measured bus voltage [mV]. */
};

/* ==============================================================================
 * DRIVER INSTANCE (PRIVATE)
 * =============================================================================*/

/** @brief Setpoint handed from the slow loop to the fast loop. Private. */
struct sstep_setpoint
{
    sstep_angle_t angle;        /**< Electrical angle at the start of the slow period. */
    int32_t dangle;             /**< Angle increment per fast loop period. */
    sstep_real_t i_amp;         /**< Current amplitude [pu]. */
    sstep_real_t ff;            /**< Back-EMF feed-forward amplitude [pu of V_base]. */
    struct sstep_gain inv_vbus; /**< V_base / measured bus voltage. */
};

/**
 * @brief Motion generator state. Private. Integer arithmetic in both numeric backends:
 *        positions in 1/65536 microstep, velocities in 1/65536 microstep per slow loop
 *        period, accelerations in 1/65536 microstep per period squared.
 */
struct sstep_motion
{
    int64_t pos;          /**< Position [Q16 microsteps]. */
    int64_t target;       /**< Move target [Q16 microsteps]. */
    int64_t brake_vmax;   /**< Braking distance from v_max [Q16 microsteps]. */
    int32_t vel;          /**< Velocity [Q16 microsteps / period]. */
    int32_t v_run;        /**< Requested velocity of a RUN command. */
    int32_t v_max;        /**< Velocity limit. */
    int32_t acc;          /**< Acceleration limit [Q16 microsteps / period^2]. */
    int32_t dec;          /**< Deceleration limit. */
    uint8_t mode;         /**< 0 hold, 1 move, 2 run. */
};

/**
 * @brief Driver instance. Allocated by the application (static, stack or pool) and
 *        initialised by sstep_init(). The members are private: their layout may change
 *        in any release; use the functions of this header only.
 */
struct sstep
{
    struct sstep_config cfg;          /**< Copy of the configuration. */

    /* Constants derived by sstep_init() */
    struct sstep_gain kp;             /**< Proportional gain [pu V / pu A]. */
    struct sstep_gain ki;             /**< Integral gain per fast loop period. */
    struct sstep_gain kb;             /**< Anti-windup gain Ts / Tt. */
    struct sstep_gain r_pu;           /**< Phase resistance [pu] (voltage drive). */
    struct sstep_gain adc_i;          /**< Current per ADC count [pu]. */
    struct sstep_gain adc_v;          /**< Bus voltage per ADC count [pu]. */
    struct sstep_gain ff_gain;        /**< Feed-forward per microstep/s [pu]. */
    sstep_real_t i_run;               /**< Run current [pu]. */
    sstep_real_t i_hold;              /**< Hold current [pu]. */
    uint16_t i_trip_counts;           /**< Overcurrent trip [ADC counts from zero]. */
    uint16_t vbus_min_raw;            /**< Undervoltage trip [ADC counts], 0 = off. */
    uint16_t vbus_max_raw;            /**< Overvoltage trip [ADC counts], 0 = off. */
    uint16_t ratio;                   /**< Fast loop periods per slow loop period. */
    uint32_t hold_ticks;              /**< Slow periods at standstill before hold. */
    uint8_t angle_shift;              /**< 14 - log2(microsteps). */

    /* Slow loop -> fast loop hand-off (double buffer) */
    struct sstep_setpoint sp[2];      /**< Setpoints; the fast loop owns sp[sp_active]. */
    volatile uint8_t sp_active;       /**< Buffer in use by the fast loop. */
    volatile uint8_t sp_pending;      /**< 1: sp[1 - sp_active] is complete and new. */

    /* Fast loop state */
    struct sstep_setpoint cur;        /**< Setpoint in use. */
    uint16_t adv;                     /**< Angle increments applied since the hand-off. */
    sstep_integ_t integ[2];           /**< PI integrators [pu V]. */
    sstep_real_t i_meas[2];           /**< Last measured phase currents [pu]. */
    volatile uint16_t vbus_raw;       /**< Last bus voltage [ADC counts]. */
    uint16_t compare[SSTEP_LEGS_MAX]; /**< Last compare values. */

    /* Slow loop state */
    struct sstep_motion mo;           /**< Motion generator. */
    sstep_angle_t angle_offset;       /**< Electrical angle of position 0. */
    uint32_t still;                   /**< Slow periods at standstill. */
    bool hold_active;                 /**< Hold current applied. */
    struct sstep_gain inv_vbus;       /**< Last V_base / bus voltage. */

    volatile uint16_t faults;         /**< Active faults. */
    volatile enum sstep_state state;  /**< Driver state. */
};

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * =============================================================================*/

/**
 * @brief  Validate a configuration and initialise a driver. Outputs stay disabled.
 *         Derives the per-unit bases, the PI gains (Kp = L * wc, Ki = R * wc) and the
 *         trajectory constants; no floating point is used with the fixed point backend.
 * @param  m    Driver instance.
 * @param  cfg  Configuration; copied.
 * @retval ::SSTEP_OK              Success, state ::SSTEP_STATE_DISABLED.
 * @retval ::SSTEP_ERR_PARAM       NULL pointer, missing required callback or a field out of range.
 * @retval ::SSTEP_ERR_CONFIG      Inconsistent fields (for example i_run_ma > i_trip_ma).
 * @retval ::SSTEP_ERR_UNSUPPORTED Topology not compiled in.
 */
int sstep_init(struct sstep* m, const struct sstep_config* cfg);

/**
 * @brief  Enable the outputs and hold the current position with the hold current.
 * @param  m  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (faulted).
 */
int sstep_enable(struct sstep* m);

/**
 * @brief  Stop immediately and switch the outputs off (motor coasts, no holding torque).
 * @param  m  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_disable(struct sstep* m);

/**
 * @brief  Fast loop: convert the measurements, run the current controllers, limit the
 *         voltage, map it to the legs of the topology and write the PWM.
 *         Call from the PWM/ADC interrupt at `fast_loop_hz`. Bounded execution time,
 *         no loops over data.
 * @param  m       Driver instance.
 * @param  sample  Measurements of this period.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (only when ::SSTEP_CFG_ARG_CHECKS is 1).
 */
int sstep_fast_loop(struct sstep* m, const struct sstep_sample* sample);

/**
 * @brief  Slow loop: advance the trajectory, update the angle and current references,
 *         handle the hold current and report events. Call at `slow_loop_hz`.
 * @param  m  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_slow_loop(struct sstep* m);

/**
 * @brief  Start a move to an absolute position with the trapezoidal profile.
 *         Replaces the current move; the change is smooth (no velocity step).
 * @param  m         Driver instance.
 * @param  position  Target [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (disabled or faulted).
 */
int sstep_move_to(struct sstep* m, int32_t position);

/**
 * @brief  Start a move relative to the current target.
 * @param  m      Driver instance.
 * @param  delta  Distance [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (also when the target overflows), ::SSTEP_ERR_STATE.
 */
int sstep_move_by(struct sstep* m, int32_t delta);

/**
 * @brief  Run at a constant velocity (accelerating with the configured limits) until
 *         another command.
 * @param  m         Driver instance.
 * @param  velocity  Signed velocity [microsteps/s], |velocity| <= v_max.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE.
 */
int sstep_run(struct sstep* m, int32_t velocity);

/**
 * @brief  Decelerate to standstill with the configured deceleration and hold.
 * @param  m  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_stop(struct sstep* m);

/**
 * @brief  Redefine the current position (homing). Only at standstill.
 * @param  m         Driver instance.
 * @param  position  New value of the current position [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (moving).
 */
int sstep_set_position(struct sstep* m, int32_t position);

/**
 * @brief  Change the run and hold currents.
 * @param  m        Driver instance.
 * @param  run_ma   Peak phase current while moving [mA].
 * @param  hold_ma  Peak phase current at standstill [mA], <= run_ma.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_CONFIG (above the trip level).
 */
int sstep_set_current(struct sstep* m, uint32_t run_ma, uint32_t hold_ma);

/**
 * @brief  Change the motion limits; applies to the next command.
 * @param  m      Driver instance.
 * @param  v_max  Maximum velocity [microsteps/s].
 * @param  accel  Acceleration [microsteps/s^2].
 * @param  decel  Deceleration [microsteps/s^2]; 0 = same as accel.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_set_motion_limits(struct sstep* m, uint32_t v_max, uint32_t accel, uint32_t decel);

/**
 * @brief  Report a fault detected by the application (driver fault pin, temperature).
 *         Switches the outputs off. May be called from an interrupt.
 * @param  m       Driver instance.
 * @param  faults  Flags to raise, typically ::SSTEP_FAULT_EXTERNAL.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_report_fault(struct sstep* m, uint16_t faults);

/**
 * @brief  Clear all faults; the driver goes to ::SSTEP_STATE_DISABLED.
 * @param  m  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_clear_faults(struct sstep* m);

/**
 * @brief  Read a status snapshot in engineering units.
 * @param  m       Driver instance.
 * @param  status  Filled on success.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_get_status(const struct sstep* m, struct sstep_status* status);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SSTEP_CTRL_H */
