/**
  ******************************************************************************
  * @file    eeprom.c
  * @brief   EEPROM emulation using STM32F103C8Tx flash memory
  * @details Flash is organized as 128 pages of 512 bytes
  *          Uses last page (Page 63) at 0x0800FC00 for EEPROM storage
  ******************************************************************************
  */

#include "eeprom.h"
#include "stm32f1xx_hal.h"

/* ========== EEPROM Data Structure ========== */
static EEPROMData_t eeprom_data = {
    .magic = 0xDEADBEEF,
    .setTemp = 28,
    .crc = 0
};

/* ========== CRC16 Calculation ========== */
/**
 * @brief Calculate CRC16 checksum (CRC-CCITT)
 * @param data: Pointer to data buffer
 * @param length: Length of data in bytes
 * @retval CRC16 value
 */
uint16_t EEPROM_CRC16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
            crc &= 0xFFFF;
        }
    }
    return crc;
}

/* ========== Unlock/Lock Flash ========== */
/**
 * @brief Unlock flash memory for write operations
 */
static void Flash_Unlock(void)
{
    HAL_FLASH_Unlock();
}

/**
 * @brief Lock flash memory after write operations
 */
static void Flash_Lock(void)
{
    HAL_FLASH_Lock();
}

/* ========== Erase EEPROM Page ========== */
/**
 * @brief Erase the EEPROM flash page
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
static HAL_StatusTypeDef EEPROM_ErasePage(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = EEPROM_START_ADDR;
    EraseInitStruct.NbPages = 1;
    
    Flash_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    Flash_Lock();
    
    return status;
}

/* ========== Write to Flash ========== */
/**
 * @brief Write data to flash at specified address
 * @param address: Flash address to write to
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
static HAL_StatusTypeDef EEPROM_WriteFlash(uint32_t address, const uint8_t *data, uint32_t size)
{
    Flash_Unlock();
    
    for (uint32_t i = 0; i < size; i += 2)
    {
        uint16_t word = data[i];
        if (i + 1 < size)
        {
            word |= (data[i + 1] << 8);
        }
        
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + i, word) != HAL_OK)
        {
            Flash_Lock();
            return HAL_ERROR;
        }
    }
    
    Flash_Lock();
    return HAL_OK;
}

/* ========== Public Functions ========== */

/**
 * @brief Initialize EEPROM module and load saved setpoint
 * @retval HAL_OK if successful and valid data found, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_Init(void)
{
    /* Read EEPROM data from flash */
    EEPROMData_t *flash_data = (EEPROMData_t *)EEPROM_START_ADDR;
    
    /* Verify magic number */
    if (flash_data->magic != 0xDEADBEEF)
    {
        /* No valid data in EEPROM, use defaults */
        eeprom_data.magic = 0xDEADBEEF;
        eeprom_data.setTemp = 28;
        eeprom_data.crc = EEPROM_CRC16((const uint8_t *)&eeprom_data, 
                                        sizeof(eeprom_data) - sizeof(uint16_t));
        return HAL_ERROR;  /* Data not valid, using defaults */
    }
    
    /* Verify data integrity with CRC */
    uint16_t calculated_crc = EEPROM_CRC16((const uint8_t *)flash_data, 
                                            sizeof(EEPROMData_t) - sizeof(uint16_t));
    
    if (calculated_crc != flash_data->crc)
    {
        /* CRC mismatch, data is corrupted */
        eeprom_data.magic = 0xDEADBEEF;
        eeprom_data.setTemp = 28;
        eeprom_data.crc = EEPROM_CRC16((const uint8_t *)&eeprom_data, 
                                        sizeof(eeprom_data) - sizeof(uint16_t));
        return HAL_ERROR;
    }
    
    /* Valid data found, copy to RAM buffer */
    eeprom_data.magic = flash_data->magic;
    eeprom_data.setTemp = flash_data->setTemp;
    eeprom_data.crc = flash_data->crc;
    
    return HAL_OK;
}

/**
 * @brief Save setpoint to EEPROM (flash memory)
 * @param setTemp: Temperature setpoint to save (10-50°C)
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_SaveSetpoint(int8_t setTemp)
{
    /* Validate range */
    if (setTemp < 10 || setTemp > 50)
    {
        return HAL_ERROR;
    }
    
    /* Update RAM buffer */
    eeprom_data.magic = 0xDEADBEEF;
    eeprom_data.setTemp = setTemp;
    eeprom_data.crc = EEPROM_CRC16((const uint8_t *)&eeprom_data, 
                                    sizeof(eeprom_data) - sizeof(uint16_t));
    
    /* Erase the flash page */
    if (EEPROM_ErasePage() != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    /* Write the new data */
    return EEPROM_WriteFlash(EEPROM_START_ADDR, (const uint8_t *)&eeprom_data, 
                             sizeof(EEPROMData_t));
}

/**
 * @brief Load setpoint from EEPROM
 * @param pSetTemp: Pointer to store loaded setpoint
 * @retval HAL_OK if successful and data valid, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_LoadSetpoint(int8_t *pSetTemp)
{
    if (pSetTemp == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Read from flash */
    EEPROMData_t *flash_data = (EEPROMData_t *)EEPROM_START_ADDR;
    
    /* Verify magic number */
    if (flash_data->magic != 0xDEADBEEF)
    {
        return HAL_ERROR;
    }
    
    /* Verify CRC */
    uint16_t calculated_crc = EEPROM_CRC16((const uint8_t *)flash_data, 
                                            sizeof(EEPROMData_t) - sizeof(uint16_t));
    
    if (calculated_crc != flash_data->crc)
    {
        return HAL_ERROR;
    }
    
    /* Validate temperature range */
    if (flash_data->setTemp < 10 || flash_data->setTemp > 50)
    {
        return HAL_ERROR;
    }
    
    *pSetTemp = flash_data->setTemp;
    return HAL_OK;
}

/**
 * @brief Erase EEPROM (write default values)
 * @param defaultSetTemp: Default setpoint to write
 * @retval HAL_OK if successful, HAL_ERROR otherwise
 */
HAL_StatusTypeDef EEPROM_Erase(int8_t defaultSetTemp)
{
    /* Validate range */
    if (defaultSetTemp < 10 || defaultSetTemp > 50)
    {
        return HAL_ERROR;
    }
    
    /* Update RAM buffer with defaults */
    eeprom_data.magic = 0xDEADBEEF;
    eeprom_data.setTemp = defaultSetTemp;
    eeprom_data.crc = EEPROM_CRC16((const uint8_t *)&eeprom_data, 
                                    sizeof(eeprom_data) - sizeof(uint16_t));
    
    /* Erase the flash page */
    if (EEPROM_ErasePage() != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    /* Write default data */
    return EEPROM_WriteFlash(EEPROM_START_ADDR, (const uint8_t *)&eeprom_data, 
                             sizeof(EEPROMData_t));
}
