/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public API: library version, public types and all public functions.
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

#ifndef SSTEP_H
#define SSTEP_H

/* ==============================================================================
 * VERSION
 * =============================================================================*/

/* Normally defined by cmake/version.cmake from the latest vX.Y.Z tag; these
 * fallbacks are used when the library is built without CMake. Keep them in sync
 * with SSTEP_DEFAULT_VERSION and CHANGELOG.md. */

#ifndef SSTEP_VERSION_MAJOR
#define SSTEP_VERSION_MAJOR 1
#endif

#ifndef SSTEP_VERSION_MINOR
#define SSTEP_VERSION_MINOR 0
#endif

#ifndef SSTEP_VERSION_PATCH
#define SSTEP_VERSION_PATCH 0
#endif

#ifndef SSTEP_VERSION
#define SSTEP_VERSION "1.0.0"
#endif

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "sstep_config.h"
#include "sstep_defs.h"
#include "sstep_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup sstep_api Stepper-C API
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
 *
 * **Initialization.** Every function except sstep_init() returns ::SSTEP_ERR_STATE for
 * an instance without a successful sstep_init() (a zeroed instance, or one whose last
 * sstep_init() failed in the derivation of the constants), and touches no hardware.
 * sstep_fast_loop() checks this only when ::SSTEP_CFG_ARG_CHECKS is 1.
 * @{
 */

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * =============================================================================*/

/**
 * @brief  Validate a configuration and initialise a driver. Outputs stay disabled.
 *         Derives the per-unit bases, the PI gains (Kp = L * wc, Ki = R * wc) and the
 *         trajectory constants; no floating point is used with the fixed point backend.
 * @param  drv  Driver instance.
 * @param  cfg  Configuration; copied.
 * @retval ::SSTEP_OK              Success, state ::SSTEP_STATE_DISABLED.
 * @retval ::SSTEP_ERR_PARAM       NULL pointer, missing required callback or a field out of range.
 * @retval ::SSTEP_ERR_CONFIG      Inconsistent fields (for example i_run_ma > i_trip_ma).
 * @retval ::SSTEP_ERR_UNSUPPORTED Topology not compiled in.
 */
int sstep_init(struct sstep* drv, const struct sstep_config* cfg);

/**
 * @brief  Enable the outputs and hold the current position with the hold current.
 * @param  drv  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (faulted).
 */
int sstep_enable(struct sstep* drv);

/**
 * @brief  Stop immediately and switch the outputs off (motor coasts, no holding torque).
 * @param  drv  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_disable(struct sstep* drv);

/**
 * @brief  Fast loop: convert the measurements, run the current controllers, limit the
 *         voltage, map it to the legs of the topology and write the PWM.
 *         Call from the PWM/ADC interrupt at `fast_loop_hz`. Bounded execution time,
 *         no loops over data.
 * @param  drv     Driver instance.
 * @param  sample  Measurements of this period.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (only when ::SSTEP_CFG_ARG_CHECKS is 1).
 */
int sstep_fast_loop(struct sstep* drv, const struct sstep_sample* sample);

/**
 * @brief  Slow loop: advance the trajectory, update the angle and current references,
 *         handle the hold current and report events. Call at `slow_loop_hz`.
 * @param  drv  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_slow_loop(struct sstep* drv);

/**
 * @brief  Start a move to an absolute position with the trapezoidal profile.
 *         Replaces the current move; the change is smooth (no velocity step).
 * @param  drv       Driver instance.
 * @param  position  Target [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (disabled or faulted).
 */
int sstep_move_to(struct sstep* drv, int32_t position);

/**
 * @brief  Start a move relative to the current target.
 * @param  drv    Driver instance.
 * @param  delta  Distance [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM (also when the target overflows), ::SSTEP_ERR_STATE.
 */
int sstep_move_by(struct sstep* drv, int32_t delta);

/**
 * @brief  Run at a constant velocity (accelerating with the configured limits) until
 *         another command.
 * @param  drv       Driver instance.
 * @param  velocity  Signed velocity [microsteps/s], |velocity| <= v_max.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE.
 */
int sstep_run(struct sstep* drv, int32_t velocity);

/**
 * @brief  Decelerate to standstill with the configured deceleration and hold.
 * @param  drv  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_stop(struct sstep* drv);

/**
 * @brief  Redefine the current position (homing). Only at standstill.
 * @param  drv       Driver instance.
 * @param  position  New value of the current position [microsteps].
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_STATE (moving).
 */
int sstep_set_position(struct sstep* drv, int32_t position);

/**
 * @brief  Change the run and hold currents.
 * @param  drv      Driver instance.
 * @param  run_ma   Peak phase current while moving [mA].
 * @param  hold_ma  Peak phase current at standstill [mA], <= run_ma.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM, ::SSTEP_ERR_CONFIG (above the trip level).
 */
int sstep_set_current(struct sstep* drv, uint32_t run_ma, uint32_t hold_ma);

/**
 * @brief  Change the motion limits; they apply from the next sstep_slow_loop(), also to a
 *         running command.
 * @param  drv    Driver instance.
 * @param  v_max  Maximum velocity [microsteps/s].
 * @param  accel  Acceleration [microsteps/s^2].
 * @param  decel  Deceleration [microsteps/s^2]; 0 = same as accel.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_set_motion_limits(struct sstep* drv, uint32_t v_max, uint32_t accel, uint32_t decel);

/**
 * @brief  Report a fault detected by the application (driver fault pin, temperature).
 *         Switches the outputs off. May be called from an interrupt.
 * @param  drv     Driver instance.
 * @param  faults  Flags to raise, typically ::SSTEP_FAULT_EXTERNAL.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_report_fault(struct sstep* drv, uint16_t faults);

/**
 * @brief  Clear all faults; the driver goes to ::SSTEP_STATE_DISABLED.
 * @param  drv  Driver instance.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_clear_faults(struct sstep* drv);

/**
 * @brief  Read a status snapshot in engineering units.
 * @param  drv     Driver instance.
 * @param  status  Filled on success.
 * @retval ::SSTEP_OK, ::SSTEP_ERR_PARAM.
 */
int sstep_get_status(const struct sstep* drv, struct sstep_status* status);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_H */
