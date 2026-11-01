/*
 * Put before every C snippet of the documentation by tools/docs/check_snippets.py:
 * the library and the board functions the snippets stand in for.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sstep.h"

/* Board functions used by the snippets of docs/ (declare each one they call). */
void board_init(void);
void board_pwm_set(const uint16_t compare[SSTEP_LEGS_MAX]);
void board_pwm_set_axis(uint32_t axis, const uint16_t compare[SSTEP_LEGS_MAX]);
void board_gates_enable(bool enable);
uint16_t board_adc_phase_a(void);
uint16_t board_adc_phase_b(void);
uint16_t board_adc_vbus(void);
bool board_tick_1ms(void);
bool board_driver_fault(void);
void board_led(bool on);
void board_log_stall(int32_t velocity, int32_t rotor_velocity, int32_t load_angle_cdeg,
                     uint8_t level_pct);
