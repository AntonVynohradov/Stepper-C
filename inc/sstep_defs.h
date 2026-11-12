/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public constants: status codes, topologies, power stage legs, faults, states.
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


#ifndef SSTEP_DEFS_H
#define SSTEP_DEFS_H

/**
 * @defgroup sstep_defs Definitions
 * @brief    Constants shared by the public API.
 *
 * **Units.** Configuration values are integers in engineering units (milliohm,
 * microhenry, milliampere, millivolt, hertz, nanosecond) so that sstep_init() needs no
 * floating point with the fixed point backend. The unit is part of every field name
 * (`r_mohm`, `l_uh`...). Positions are in microsteps, velocities in microsteps per second.
 *
 * **Sign convention.** A positive phase voltage drives a positive phase current;
 * positive velocity increases the position. Phase A leads phase B by 90 electrical
 * degrees for positive motion.
 * @{
 */

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdint.h>

/* ==============================================================================
 * STATUS CODES
 * =============================================================================*/

/** @brief Success. */
#define SSTEP_OK 0
/** @brief NULL pointer or a value outside its documented range. */
#define SSTEP_ERR_PARAM (-1)
/** @brief Configuration is consistent field by field but not as a whole
 *         (for example rated current above the power stage limit). */
#define SSTEP_ERR_CONFIG (-2)
/** @brief Call not allowed in the current state (for example a move while faulted). */
#define SSTEP_ERR_STATE (-3)
/** @brief Topology or self-test not compiled in (see ::SSTEP_CFG_TOPOLOGY_FULL_BRIDGE,
 *         ::SSTEP_CFG_SELFTEST). */
#define SSTEP_ERR_UNSUPPORTED (-4)
/** @brief The last bus voltage sample is outside vbus_min_mv .. vbus_max_mv: the outputs
 *         stay off, no fault is latched (sstep_enable(), sstep_selftest()). */
#define SSTEP_ERR_VBUS (-5)

/* ==============================================================================
 * POWER STAGE
 * =============================================================================*/

/** @brief Number of half-bridge legs of the largest supported topology. */
#define SSTEP_LEGS_MAX 4U

/** @brief Power stage topology. */
enum sstep_topology
{
    /** Two full H-bridges, 4 legs. Phase A between legs 0 and 1, phase B between
     *  legs 2 and 3. Phase voltage range: -Vbus..+Vbus for each phase independently. */
    SSTEP_TOPOLOGY_FULL_BRIDGE = 0,
    /** Three half-bridges, 3 legs. Phase A between legs 0 and 2, phase B between
     *  legs 1 and 2; leg 2 is shared and carries iA + iB. Phase voltages are limited
     *  by |vA|, |vB|, |vA - vB| <= Vbus (sinusoidal amplitude about Vbus / sqrt(2)). */
    SSTEP_TOPOLOGY_SHARED_LEG = 1
};

/* ==============================================================================
 * FAULTS
 * =============================================================================*/

/** @brief Fault flags reported by sstep_get_status() and the fault callback.
 *         A fault switches the outputs off; sstep_clear_faults() re-arms the driver. */
#define SSTEP_FAULT_NONE 0x0000U
/** @brief Measured phase current above the configured trip level. */
#define SSTEP_FAULT_OVERCURRENT 0x0001U
/** @brief Bus voltage above the configured maximum. */
#define SSTEP_FAULT_OVERVOLTAGE 0x0002U
/** @brief Bus voltage below the configured minimum. */
#define SSTEP_FAULT_UNDERVOLTAGE 0x0004U
/** @brief A hardware callback returned an error. */
#define SSTEP_FAULT_HAL 0x0008U
/** @brief Reported by the application through sstep_report_fault() (driver fault pin,
 *         over-temperature...). */
#define SSTEP_FAULT_EXTERNAL 0x0010U
/** @brief The rotor does not follow the commanded electrical angle: lost synchronism in
 *         motion, or rotor motion while the command is at rest (sensorless back-EMF
 *         observer, enabled with ::sstep_ctrl_cfg::stall_time_ms). */
#define SSTEP_FAULT_STALL 0x0020U
/** @brief sstep_selftest() found the power stage or the motor wiring faulty; the result is
 *         in ::sstep_status::selftest. */
#define SSTEP_FAULT_SELFTEST 0x0040U

/* ==============================================================================
 * STATES
 * =============================================================================*/

/** @brief Driver state. */
enum sstep_state
{
    SSTEP_STATE_DISABLED = 0, /**< Outputs off (all legs high impedance). */
    SSTEP_STATE_HOLDING = 1,  /**< Outputs on, position held with the hold current. */
    SSTEP_STATE_MOVING = 2,   /**< A move or a velocity command is executing. */
    SSTEP_STATE_FAULT = 3     /**< Outputs off after a fault, see sstep_clear_faults(). */
};

/* ==============================================================================
 * SELF-TEST
 * =============================================================================*/

/** @brief Result of sstep_selftest(), in ::sstep_status::selftest. */
enum sstep_selftest_result
{
    SSTEP_SELFTEST_NONE = 0,     /**< Not run since sstep_init(), or stopped by sstep_disable(). */
    SSTEP_SELFTEST_RUNNING = 1,  /**< The fast loop is running the test. */
    SSTEP_SELFTEST_PASS = 2,     /**< Both phases carry the test current as expected. */
    SSTEP_SELFTEST_LEAK = 3,     /**< Current with every leg low: a switch conducts. */
    SSTEP_SELFTEST_OPEN = 4,     /**< No current at the largest test duty cycle: phase open,
                                      motor not connected or a leg dead. */
    SSTEP_SELFTEST_SHORT = 5,    /**< Current too high, or reached at a duty cycle below the
                                      resistance of the phase: short circuit. */
    SSTEP_SELFTEST_POLARITY = 6, /**< Current of the wrong sign: the sign of adc_i_span. */
    SSTEP_SELFTEST_WRONG_PHASE = 7, /**< The other phase carries half the test current or more:
                                         a phase connected to the wrong legs. */
    SSTEP_SELFTEST_ABORTED = 8      /**< A trip of the fast loop (bus voltage) ended the test. */
};

/** @} */

#endif /* SSTEP_DEFS_H */
