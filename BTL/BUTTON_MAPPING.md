# Button Mapping & Control Reference

## Physical Button Pins (STM32F103C8Tx)

| Button | MCU Pin | GPIO Port | GPIO Pin | Function |
|--------|---------|-----------|----------|----------|
| **UP** | PA2 | GPIOA | GPIO_PIN_2 | Increase Temperature Setpoint |
| **DOWN** | PA3 | GPIOA | GPIO_PIN_3 | Decrease Temperature Setpoint |
| **SET** | PA4 | GPIOA | GPIO_PIN_4 | Toggle Settings/Normal Mode |
| **POWER** | PA5 | GPIOA | GPIO_PIN_5 | Toggle System ON/OFF |

## Button Behavior by System Mode

### OFF Mode (Power OFF)
```
Button Pressed → Action
━━━━━━━━━━━━━━━━━━━━━━━━━━
POWER          → Switch to NORMAL mode, fan OFF
UP             → No effect
DOWN           → No effect
SET            → No effect

Display: "T:25.3 C S:28 / M:OFF F:OFF"
```

### NORMAL Mode (Temperature Control Active)
```
Button Pressed → Action
━━━━━━━━━━━━━━━━━━━━━━━━━━
POWER          → Switch to OFF mode, fan OFF
UP             → No effect
DOWN           → No effect
SET            → Switch to SETTING mode, fan OFF

Display: "T:25.3 C S:28 / M:NORMAL F:ON"
          (Fan controlled by hysteresis)
```

### SETTING Mode (Adjust Temperature)
```
Button Pressed → Action
━━━━━━━━━━━━━━━━━━━━━━━━━━
POWER          → Switch to OFF mode, fan OFF
UP             → Increase setTemp (max 50°C)
DOWN           → Decrease setTemp (min 10°C)
SET            → Return to NORMAL mode, resume control

Display: "T:25.3 C S:28 / M:SETTING F:OFF"
          (Fan forced OFF during settings)
```

## Debounce Algorithm

**Debounce Time:** ~250ms (5 samples × 50ms cycle)

```
Time    Button   Count   State   Action
────────────────────────────────────────────
0ms     Pressed  1       
50ms    Pressed  2       
100ms   Pressed  3       
150ms   Pressed  4       
200ms   Pressed  5       ✓      Press detected!
250ms   Released 0       Released
```

This prevents accidental double-presses from electrical noise or contact bounce.

## State Transition Diagram

```
                    ┌─────────────┐
                    │   OFF MODE  │
                    │  F:OFF S:28 │
                    └──────┬──────┘
                           │
                    POWER  │ POWER
                           ↓
                    ┌─────────────┐
                    │ NORMAL MODE │
                    │  F:ON S:28  │
                    └──────┬──────┘
                           │
                    SET    │ SET/POWER
                           ↓
                    ┌─────────────┐
                    │SETTING MODE │
                    │  F:OFF S:28 │ ← UP/DOWN adjust setTemp
                    └──────┬──────┘
                           │ SET (confirm)
                           ↓ (return to NORMAL)
```

## Code Integration Points

### Button Pin Definitions (main.h)
```c
#define Up_Pin          GPIO_PIN_2
#define Up_GPIO_Port    GPIOA
#define Down_Pin        GPIO_PIN_3
#define Down_GPIO_Port  GPIOA
#define Set_Pin         GPIO_PIN_4
#define Set_GPIO_Port   GPIOA
#define Power_Pin       GPIO_PIN_5
#define Power_GPIO_Port GPIOA
```

### Button Reading (app_tasks.c - Task_Input)
```c
button_reads[0] = HAL_GPIO_ReadPin(Up_GPIO_Port, Up_Pin);
button_reads[1] = HAL_GPIO_ReadPin(Down_GPIO_Port, Down_Pin);
button_reads[2] = HAL_GPIO_ReadPin(Set_GPIO_Port, Set_Pin);
button_reads[3] = HAL_GPIO_ReadPin(Power_GPIO_Port, Power_Pin);
```

### Button Handling (app_tasks.c - Handle_Button_Press)
```c
case 0:   // UP button
case 1:   // DOWN button
case 2:   // SET button
case 3:   // POWER button
```

## Testing Checklist

- [ ] UP button increases setTemp in SETTING mode (10-50°C range)
- [ ] DOWN button decreases setTemp in SETTING mode
- [ ] SET button toggles between NORMAL and SETTING modes
- [ ] POWER button toggles between OFF and NORMAL modes
- [ ] No accidental presses due to bouncing (debounce working)
- [ ] Button response feels responsive (~250ms debounce)
- [ ] Multiple rapid presses handled correctly
- [ ] Setpoint remains saved after exiting SETTING mode

## GPIO Hardware Configuration

### Input Button Pins
- **Mode:** GPIO_MODE_INPUT (standard input)
- **Pull:** GPIO_PULLDOWN (active HIGH when pressed)
- **Speed:** GPIO_SPEED_FREQ_LOW (input doesn't need high speed)

### Connection Pattern
```
Button Module (typical)
┌─────────────┐
│   Button    │ ──── PA2/PA3/PA4/PA5 (MCU Input)
│   Ground    │ ──── GND
└─────────────┘

When button pressed:
  GPIO reads: GPIO_PIN_SET (1)
  
When button released:
  GPIO reads: GPIO_PIN_RESET (0)
```

---

**Last Updated:** 2025-11-28  
**Project:** BTL Thermostat Control System  
**Status:** Ready for Testing
