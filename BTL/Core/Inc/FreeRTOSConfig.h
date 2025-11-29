/*
 * FreeRTOS Kernel V10.4.1
 * Configuration file for STM32F103C8Tx with CMSIS-RTOS v2
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * IMPORTANT: The configured value for configTOTAL_HEAP_SIZE is set to 20KB
 * which matches the STM32F103C8's SRAM. Be careful not to exceed this.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                    1
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) 12000 )  /* 12KB heap out of 20KB total RAM */
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1

/* Kernel tick frequency (must match HAL SysTick config) */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )

/* Scheduling configuration */
#define configMAX_PRIORITIES                    ( 5 )  /* osPriorityIdle(0), osPriorityLow(1), osPriorityNormal(2), osPriorityHigh(3), osPriorityRealtime(4) */
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 128 )
#define configCHECK_FOR_STACK_OVERFLOW          0

/* Hook functions */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0

/* Mutex/Semaphore configuration */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0

/* Queue configuration */
#define configUSE_QUEUE_SETS                    0

/* Software timer configuration */
#define configUSE_TIMERS                        0

/* Time management */
#define configUSE_TIME_SLICING                  1

/* Debug and assert configuration */
#define configASSERT( x )                       ( ( x ) ? ( void ) 0 : configASSERT_CALLED() )
extern void configASSERT_CALLED( void );

/* Trace/stats configuration */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0

/* Co-routine configuration */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Set the following definitions to 1 to include the API function, or zero to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        0
#define INCLUDE_xTimerPendFunctionCall          0

/* STM32 specific configuration */
#define configCPU_CLOCK_HZ                      ( 72000000UL )
#define configSYSTICK_CLOCK_HZ                  ( 72000000UL )
#define configSYSTICK_USE_LOW_POWER_CLOCK       0

/* Cortex-M3 specific configuration */
#define configKERNEL_INTERRUPT_PRIORITY         15
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191  /* configKERNEL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) */
#define configPRIO_BITS                         4

#endif /* FREERTOS_CONFIG_H */
