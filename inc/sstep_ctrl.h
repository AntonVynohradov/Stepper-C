/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public types: configuration, hardware callbacks, measurements, driver instance.
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

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sstep_config.h"
#include "sstep_defs.h"

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
 * DRIVER INSTANCE
 * =============================================================================*/

/**
 * @brief Bytes of the private driver state that follow the configuration copy. Covers
 *        both numeric backends on 32-bit (Cortex-M) and 64-bit hosts; sstep_ctrl.c
 *        checks it at compile time.
 */
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
#define SSTEP_STATE_BYTES 200U
#else
#define SSTEP_STATE_BYTES 224U
#endif

/** @brief Size of ::sstep in 64-bit words. */
#define SSTEP_INSTANCE_WORDS ((sizeof(struct sstep_config) + SSTEP_STATE_BYTES + 7U) / 8U)

/**
 * @brief Driver instance. Allocated by the application (static, stack or pool) and
 *        initialised by sstep_init(). The content is private to the library: use the
 *        functions of sstep.h only.
 */
struct sstep
{
    uint64_t opaque[SSTEP_INSTANCE_WORDS]; /**< Private driver state. */
};

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_CTRL_H */
