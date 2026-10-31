/*
 * Minimal FreeRTOS declarations for tools/docs/check_snippets.py: only what the
 * documentation snippets use. Not FreeRTOS.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SNIPPET_FREERTOS_H
#define SNIPPET_FREERTOS_H

#include <stdint.h>

typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

#endif
