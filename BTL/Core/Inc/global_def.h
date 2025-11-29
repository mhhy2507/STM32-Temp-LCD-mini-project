#ifndef GLOBAL_DEF_H_
#define GLOBAL_DEF_H_

#include "stm32f1xx_hal.h"

/* ========== System State Structure ========== */
typedef struct {
    float currentTemp;      // Current temperature from DS18B20
    int8_t setTemp;         // Desired temperature set by user
    uint8_t isFanOn;        // Fan status (1: ON, 0: OFF)
    uint8_t mode;           // 0: OFF, 1: NORMAL, 2: SETTING
    uint8_t button_up;      // Button UP state (PA2)
    uint8_t button_down;    // Button DOWN state (PA3)
    uint8_t button_set;     // Button SET state (PA4)
    uint8_t button_power;   // Button POWER state (PA5)
} ThermostatState_t;

/* ========== Global Variables ========== */
extern ThermostatState_t thermostat_state;

#endif /* GLOBAL_DEF_H_ */
