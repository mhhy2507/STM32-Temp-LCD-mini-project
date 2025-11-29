#ifndef DS18B20_H_
#define DS18B20_H_

#include "stm32f1xx_hal.h"

// Cấu hình chân GPIO (Sửa ở đây nếu đổi chân)
#define DS18B20_PORT GPIOB
#define DS18B20_PIN  GPIO_PIN_13

void DS18B20_Init_MicroTimer(void); // Bắt buộc gọi hàm này 1 lần đầu chương trình
uint8_t DS18B20_Start(void);
void DS18B20_Write(uint8_t data);
uint8_t DS18B20_Read(void);
float DS18B20_GetTemp(void);

#endif /* DS18B20_H_ */
