#ifndef APP_TASKS_H_
#define APP_TASKS_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"

/* ========== Task Function Prototypes ========== */

/**
 * @brief Task_Sensor - Read temperature from DS18B20
 * Period: 500ms
 * Priority: Normal
 */
void Task_Sensor(void);

/**
 * @brief Task_Input - Read and debounce button inputs
 * Period: 50ms
 * Priority: High
 */
void Task_Input(void);

/**
 * @brief Task_Control - Control fan based on temperature with hysteresis
 * Period: 100ms
 * Priority: High
 */
void Task_Control(void);

/**
 * @brief Task_Display - Update LCD display
 * Period: 200ms
 * Priority: Low
 */
void Task_Display(void);

/* ========== Task Scheduler ========== */
void Task_Scheduler_Init(void);
void Task_Scheduler_Run(void);

#endif /* APP_TASKS_H_ */
