#include "app_tasks.h"
#include "global_def.h"
#include "main.h"
#include "liquidcrystal_i2c.h"
#include "DS18B20.h"
#include "eeprom.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ========== Task Timing Control ========== */
static uint32_t task_sensor_last_time = 0;
static uint32_t task_input_last_time = 0;
static uint32_t task_control_last_time = 0;
static uint32_t task_display_last_time = 0;

/* ========== Debounce Variables ========== */
static uint8_t button_press_count[4] = {0};  // Debounce counters for 4 buttons
static uint8_t button_state[4] = {0};        // Current state of 4 buttons
#define DEBOUNCE_COUNT 3                    // Number of checks to confirm button press

/* ========== Forward Declarations ========== */
static void Button_Debounce(void);
static void Handle_Button_Press(uint8_t button_id);

/**
 * @brief Task_Sensor - Read temperature from DS18B20 sensor
 * Runs every 500ms at Normal priority
 */
void Task_Sensor(void)
{
  uint32_t current_time = HAL_GetTick();
  
  if ((current_time - task_sensor_last_time) >= 500)
  {
    task_sensor_last_time = current_time;
    
    /* Start temperature conversion */
    DS18B20_Start();
    DS18B20_Write(0xCC);  // Skip ROM command
    DS18B20_Write(0x44);  // Convert T command
    
    /* Wait for conversion (9-bit: ~187.5ms, 10-bit: ~375ms, 12-bit: 750ms) */
    HAL_Delay(400);  // Conservative delay for 12-bit
    
    /* Read temperature value */
    float temp = DS18B20_GetTemp();
    
    /* Update global state */
    thermostat_state.currentTemp = temp;
  }
}

/**
 * @brief Task_Input - Read and debounce button inputs
 * Runs every 50ms at High priority
 */
void Task_Input(void)
{
  uint32_t current_time = HAL_GetTick();
  
  if ((current_time - task_input_last_time) >= 50)
  {
    task_input_last_time = current_time;
    
    /* Perform button debouncing */
    Button_Debounce();
  }
}

/**
 * @brief Task_Control - Control fan based on temperature using hysteresis
 * Runs every 100ms at High priority
 * Hysteresis logic:
 *   - Turn ON if currentTemp >= setTemp
 *   - Turn OFF if currentTemp <= setTemp - 1.0
 */
void Task_Control(void)
{
  uint32_t current_time = HAL_GetTick();
  
  if ((current_time - task_control_last_time) >= 100)
  {
    task_control_last_time = current_time;
    
    /* Only control fan if system is in NORMAL mode */
    if (thermostat_state.mode == 1)  // NORMAL mode
    {
      float current = thermostat_state.currentTemp;
      float setpoint = thermostat_state.setTemp;
      
      /* Hysteresis control logic */
      if (current >= setpoint && !thermostat_state.isFanOn)
      {
        /* Turn ON fan when temp >= setpoint */
        thermostat_state.isFanOn = 1;
        HAL_GPIO_WritePin(Fan_in_GPIO_Port, Fan_in_Pin, GPIO_PIN_SET);
      }
      else if (current <= (setpoint - 1.0f) && thermostat_state.isFanOn)
      {
        /* Turn OFF fan when temp <= setpoint - 1.0°C */
        thermostat_state.isFanOn = 0;
        HAL_GPIO_WritePin(Fan_in_GPIO_Port, Fan_in_Pin, GPIO_PIN_RESET);
      }
    }
    else if (thermostat_state.mode == 0)  // OFF mode
    {
      /* Always turn off fan when system is OFF */
      thermostat_state.isFanOn = 0;
      HAL_GPIO_WritePin(Fan_in_GPIO_Port, Fan_in_Pin, GPIO_PIN_RESET);
    }
  }
}

/**
 * @brief Task_Display - Update LCD display with current state
 * Runs every 200ms at Low priority
 */
void Task_Display(void)
{
  uint32_t current_time = HAL_GetTick();
  char buffer[17];  // 16 chars + null terminator
  
  if ((current_time - task_display_last_time) >= 200)
  {
    task_display_last_time = current_time;
    
    /* Line 0: Display current temperature */
    lcdSetCursor(0, 0);
    sprintf(buffer, "T:%.2f C S:%d",
            thermostat_state.currentTemp, 
            thermostat_state.setTemp);
    lcdWriteString(buffer);
    
    /* Line 1: Display mode and fan status */
    lcdSetCursor(1, 0);
    
    const char *mode_str;
    if (thermostat_state.mode == 0)
      mode_str = "OFF";
    else if (thermostat_state.mode == 1)
      mode_str = "NORMAL";
    else
      mode_str = "SETTING";
    
    const char *fan_str = thermostat_state.isFanOn ? "ON " : "OFF";
    
    sprintf(buffer, "M:%s F:%s    ", mode_str, fan_str);
    lcdWriteString(buffer);
  }
}

/**
 * @brief Button debounce handler
 * Polls all 4 buttons and updates their states with debouncing
 */
static void Button_Debounce(void)
{
  /* Button mapping: 0=UP(PA2), 1=DOWN(PA3), 2=SET(PA4), 3=POWER(PA5) */
  GPIO_PinState button_reads[4];
  
  button_reads[0] = HAL_GPIO_ReadPin(Up_GPIO_Port, Up_Pin);
  button_reads[1] = HAL_GPIO_ReadPin(Down_GPIO_Port, Down_Pin);
  button_reads[2] = HAL_GPIO_ReadPin(Set_GPIO_Port, Set_Pin);
  button_reads[3] = HAL_GPIO_ReadPin(Power_GPIO_Port, Power_Pin);
  
  /* Debounce each button */
  for (uint8_t i = 0; i < 4; i++)
  {
    if (button_reads[i] == GPIO_PIN_SET)  /* Button pressed */
    {
      button_press_count[i]++;
      
      /* If button is pressed consistently for DEBOUNCE_COUNT cycles */
      if (button_press_count[i] >= DEBOUNCE_COUNT)
      {
        if (!button_state[i])  /* Edge detection: was not pressed before */
        {
          button_state[i] = 1;  /* Mark as pressed */
          Handle_Button_Press(i);  /* Handle the press */
        }
      }
    }
    else  /* Button not pressed */
    {
      button_press_count[i] = 0;
      button_state[i] = 0;
    }
  }
}

/**
 * @brief Handle button press events
 * @param button_id: 0=UP, 1=DOWN, 2=SET, 3=POWER
 */
static void Handle_Button_Press(uint8_t button_id)
{
  switch (button_id)
  {
    case 0:  /* UP button (PA2) - Increase setTemp */
      if (thermostat_state.mode == 2 && thermostat_state.setTemp < 50)
      {
        thermostat_state.setTemp++;
        /* Save to EEPROM */
        EEPROM_SaveSetpoint(thermostat_state.setTemp);
      }
      break;
      
    case 1:  /* DOWN button (PA3) - Decrease setTemp */
      if (thermostat_state.mode == 2 && thermostat_state.setTemp > 10)
      {
        thermostat_state.setTemp--;
        /* Save to EEPROM */
        EEPROM_SaveSetpoint(thermostat_state.setTemp);
      }
      break;
      
    case 2:  /* SET button (PA4) - Toggle SETTING mode */
      if (thermostat_state.mode == 1)
      {
        thermostat_state.mode = 2;  /* Enter SETTING mode */
      }
      else if (thermostat_state.mode == 2)
      {
        thermostat_state.mode = 1;  /* Exit SETTING mode */
      }
      break;
      
    case 3:  /* POWER button (PA5) - Toggle ON/OFF */
      if (thermostat_state.mode == 0)
      {
        thermostat_state.mode = 1;  /* Turn ON - enter NORMAL mode */
      }
      else
      {
        thermostat_state.mode = 0;  /* Turn OFF */
      }
      break;
  }
}

/**
 * @brief Task Scheduler Initialization
 * Initialize task timing variables
 */
void Task_Scheduler_Init(void)
{
  uint32_t current_time = HAL_GetTick();
  task_sensor_last_time = current_time;
  task_input_last_time = current_time;
  task_control_last_time = current_time;
  task_display_last_time = current_time;
}

/**
 * @brief Task Scheduler Main Loop
 * Call all tasks in sequence - they self-regulate based on timing
 * Should be called frequently (e.g., every ms or faster)
 */
void Task_Scheduler_Run(void)
{
  Task_Input();      // 50ms - HIGH priority
  Task_Control();    // 100ms - HIGH priority
  Task_Sensor();     // 500ms - NORMAL priority
  Task_Display();    // 200ms - LOW priority
}
