# EEPROM Implementation for STM32F103C8Tx Thermostat

## Overview
The setpoint temperature is now saved to the STM32F103C8Tx's internal flash memory (EEPROM emulation) and automatically loaded on power-up.

## Files Created/Modified

### 1. **Core/Inc/eeprom.h** (NEW)
- Defines EEPROM memory layout using last flash page (Page 63 at 0x0800FC00)
- Provides function prototypes for EEPROM operations
- Uses CRC16 checksum for data integrity validation
- Magic number (0xDEADBEEF) for validity checking

### 2. **Core/Src/eeprom.c** (NEW)
- Implements EEPROM emulation using STM32F1 flash memory
- Flash page size: 512 bytes (Page 63 out of 128 pages)
- Key functions:
  - `EEPROM_Init()` - Initialize and load saved setpoint
  - `EEPROM_SaveSetpoint()` - Save setpoint to flash
  - `EEPROM_LoadSetpoint()` - Load setpoint from flash
  - `EEPROM_Erase()` - Reset to default values
  - `EEPROM_CRC16()` - Calculate CRC16 checksum

### 3. **Core/Src/main.c** (MODIFIED)
- Added EEPROM initialization at startup
- Loads saved setpoint on power-up
- If EEPROM is empty or corrupted, uses default value (28°C) and saves it
- EEPROM loaded BEFORE task scheduler starts

### 4. **Core/Src/app_tasks.c** (MODIFIED)
- Added `#include "eeprom.h"`
- Modified UP button handler (PA2) - saves setpoint when increased
- Modified DOWN button handler (PA3) - saves setpoint when decreased
- Setpoint is automatically saved to EEPROM when user adjusts it in SETTING mode

## Memory Layout

```
STM32F103C8Tx Flash: 64KB total
- Pages 0-62: Program code and data
- Page 63: EEPROM storage (512 bytes)
  ├─ Offset 0x00: Magic number (0xDEADBEEF) - 4 bytes
  ├─ Offset 0x04: setTemp value - 1 byte (10-50°C)
  ├─ Offset 0x05: CRC16 checksum - 2 bytes
  └─ Offset 0x07-0x1FF: Reserved/unused
```

## Data Structure

```c
typedef struct {
    uint32_t magic;    /* 0xDEADBEEF for validation */
    int8_t setTemp;    /* Stored setpoint (10-50°C) */
    uint16_t crc;      /* CRC16 checksum */
} EEPROMData_t;  // Total: 7 bytes
```

## Operation Flow

### On System Power-Up:
1. Hardware initialization (GPIO, I2C, DS18B20)
2. LCD initialization and display "Initializing..."
3. **EEPROM_Init()** called
   - Reads last flash page (0x0800FC00)
   - Verifies magic number and CRC16
   - If valid: loads setpoint from EEPROM
   - If invalid/empty: uses default (28°C) and saves it
4. Task scheduler starts
5. Thermostat operates at saved/default setpoint

### When User Adjusts Setpoint (SETTING Mode):
1. User presses UP (PA2) or DOWN (PA3) button
2. Button debounce detects press (50ms task, 3 samples = 150ms confirmation)
3. `Handle_Button_Press()` called with button_id
4. setTemp incremented/decremented (10-50°C bounds checked)
5. **EEPROM_SaveSetpoint()** called automatically
   - Page erased (all bytes set to 0xFF)
   - New data written:
     - Magic: 0xDEADBEEF
     - setTemp: new value
     - CRC16: calculated over magic + setTemp
6. Next power-up: saved value loaded automatically

## Data Integrity Features

### CRC16 Checksum
- Polynomial: 0x1021 (CRC-CCITT)
- Initial value: 0xFFFF
- Detects data corruption in flash
- If CRC fails on load: default value used

### Magic Number Validation
- 0xDEADBEEF indicates valid EEPROM data
- If magic doesn't match: EEPROM treated as empty
- Prevents using uninitialized flash data

### Range Validation
- Setpoint must be 10-50°C
- Invalid values rejected with HAL_ERROR return
- Prevents saving out-of-range values

## Temperature Range

```
Minimum: 10°C
Maximum: 50°C
Default: 28°C
```

## Flash Wear Considerations

- Each flash page can be erased/written ~10,000 times
- Page 63 usage: ~1 write per setpoint change
- Real-world life: >100 years at 10 changes/day
- **Note**: In production, consider EEPROM wear-leveling library for higher change frequencies

## Compilation & Build

- New files automatically included in build
- No additional HAL drivers needed (uses standard `stm32f1xx_hal_flash.h`)
- Flash operation functions use existing HAL APIs
- Compatible with existing STM32CubeIDE project structure

## Testing the Feature

1. **Initial Power-Up**:
   - Default setpoint 28°C should load
   - LCD displays "S:28"

2. **Change Setpoint**:
   - Press SET (PA4) to enter SETTING mode
   - Press UP/DOWN to adjust (10-50°C)
   - Exit SETTING mode (press SET again)

3. **Power Cycle**:
   - Turn off device (unplug or reset)
   - Power back on
   - New setpoint should persist
   - LCD displays previously set value

4. **EEPROM Reset** (if needed):
   - Call `EEPROM_Erase(28)` to reset to default
   - Can be added to button combination (e.g., POWER + UP held)

## Error Handling

| Condition | Action | Result |
|-----------|--------|--------|
| EEPROM empty | Use default (28°C) | Saves default on first change |
| CRC mismatch | Treat as corrupted | Uses default, rebuilds valid data |
| Invalid magic | Treat as uninitialized | Uses default on next write |
| Setpoint out-of-range | Reject save | Value not changed |
| Flash write failure | Return HAL_ERROR | User continues with old value |

---

**Status**: Ready for compilation and testing
**Last Updated**: November 28, 2025
