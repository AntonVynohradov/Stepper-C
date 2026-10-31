/*
 * Minimal FreeRTOS task declarations for tools/docs/check_snippets.py. Not FreeRTOS.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SNIPPET_TASK_H
#define SNIPPET_TASK_H

TickType_t xTaskGetTickCount(void);
void vTaskDelayUntil(TickType_t* previous_wake, TickType_t increment);

#endif
