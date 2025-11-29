/**
  ******************************************************************************
  * @file    eeprom.h
  * @brief   EEPROM emulation using STM32F103C8Tx flash memory
  * @details Uses the last page of flash (Page 63) for persistent storage
  ******************************************************************************
  */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "stm32f1xx_hal.h"

/* ========== EEPROM Configuration ========== */
/* STM32F103C8Tx has 64KB flash, organized as 128 pages of 512 bytes each */
/* We use the last page (Page 63) starting at 0x0800FC00 for EEPROM */
#define EEPROM_START_ADDR    0x0800FC00UL    /* Last page (63) of flash */
#define EEPROM_PAGE_SIZE     512             /* Page size in bytes */

/* EEPROM data structure - stored in last flash page */
typedef struct {
    uint32_t magic;         /* Magic number 0xDEADBEEF for validation */
    int8_t setTemp;         /* Stored setpoint (10-50°C) */
    uint16_t crc;           /* CRC16 checksum for data integrity */
} EEPROMData_t;

/* ========== Function Prototypes ========== */

/**
 * @brief Initialize EEPROM module and load saved setpoint
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_Init(void);

/**
 * @brief Save setpoint to EEPROM (flash memory)
 * @param setTemp: Temperature setpoint to save (10-50°C)
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_SaveSetpoint(int8_t setTemp);

/**
 * @brief Load setpoint from EEPROM
 * @param pSetTemp: Pointer to store loaded setpoint
 * @retval HAL_OK if successful and data valid, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_LoadSetpoint(int8_t *pSetTemp);

/**
 * @brief Erase EEPROM (write default values)
 * @param defaultSetTemp: Default setpoint to write
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_Erase(int8_t defaultSetTemp);

/**
 * @brief Calculate CRC16 checksum
 * @param data: Pointer to data
 * @param length: Data length in bytes
 * @retval CRC16 value
 */
uint16_t EEPROM_CRC16(const uint8_t *data, uint16_t length);

#endif /* EEPROM_H_ */
