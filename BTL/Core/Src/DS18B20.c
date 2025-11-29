#include "ds18b20.h"

// --- Helper: Microsecond Delay using DWT ---
void DS18B20_Init_MicroTimer(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

// --- Helper: Set GPIO Mode ---
static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Open Drain
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

// --- DS18B20 Functions ---

uint8_t DS18B20_Start(void) {
    uint8_t response = 0;
    Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, 0);
    delay_us(480); // Reset pulse
    Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
    delay_us(80);
    if (!(HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN))) response = 1; // Presence detected
    else response = 0;
    delay_us(400);
    return response;
}

void DS18B20_Write(uint8_t data) {
    Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
    for (int i = 0; i < 8; i++) {
        if ((data & (1 << i)) != 0) { // Write 1
            Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, 0);
            delay_us(1);
            Set_Pin_Input(DS18B20_PORT, DS18B20_PIN); // Release line
            delay_us(60);
        } else { // Write 0
            Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, 0);
            delay_us(60);
            Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
        }
    }
}

uint8_t DS18B20_Read(void) {
    uint8_t value = 0;
    Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
    for (int i = 0; i < 8; i++) {
        Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, 0);
        delay_us(2);
        Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
        delay_us(10); // Wait for valid data
        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)) {
            value |= (1 << i);
        }
        delay_us(50);
    }
    return value;
}

// Hàm này CHỈ GỬI LỆNH đọc, không delay chờ (để dùng trong FreeRTOS)
// Bạn cần gọi DS18B20_Start() -> Write(0xCC) -> Write(0x44) -> osDelay(750) -> Gọi hàm này
float DS18B20_GetTemp(void) {
	// Giả sử cảm biến đã được Start conversion trước đó
	DS18B20_Start();
	DS18B20_Write(0xCC); // Skip ROM
	DS18B20_Write(0xBE); // Read Scratchpad

	uint8_t temp_l = DS18B20_Read();
	uint8_t temp_h = DS18B20_Read();
	uint16_t temp = (temp_h << 8) | temp_l;
	return (float)temp / 16.0;
}
