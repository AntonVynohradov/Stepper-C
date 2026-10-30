/*
 * Put before every C snippet of the documentation by tools/docs/check_snippets.py:
 * the library and the board functions the snippets stand in for.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sstep.h"

/* Board functions used by the snippets of docs/ (declare each one they call). */
int board_uart_write(const uint8_t* data, size_t length);
size_t board_uart_read(uint8_t* data, size_t capacity);
uint32_t board_millis(void);
