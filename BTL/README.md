### Danh sách Task (Tasks Breakdown)

#### 1. Task_Sensor (Priority: Normal)

* **Chu kỳ:** 500ms.
* **Chức năng:** - Kích hoạt DS18B20 đo nhiệt độ.
  * Đọc giá trị trả về.
  * Cập nhật vào biến `currentTemp`.

#### 2. Task_Input (Priority: High)

* **Chu kỳ:** 50ms.
* **Chức năng:**
  * Quét trạng thái 4 nút bấm.
  * Xử lý chống rung (Debounce).
  * Thay đổi `setTemp` hoặc `mode` tùy theo nút bấm.
  * Nút POWER: Chuyển đổi trạng thái Tắt/Mở hệ thống.
  * Nút SET: Vào chế độ cài đặt nhiệt độ.

#### 3. Task_Control (Priority: High)

* **Chu kỳ:** 100ms.
* **Chức năng:** So sánh nhiệt độ và điều khiển Relay theo thuật toán Hysteresis (Có độ trễ).
* **Logic:**
  * Nếu `currentTemp >= setTemp`:  **BẬT QUẠT** .
  * Nếu `currentTemp <= (setTemp - 1.0)`:  **TẮT QUẠT** .
  * Điều này giúp quạt không bị bật/tắt liên tục khi nhiệt độ dao động quanh ngưỡng.

#### 4. Task_Display (Priority: Low)

* **Chu kỳ:** 200ms.
* **Chức năng:**
  * Lấy dữ liệu từ biến chung.
  * Hiển thị thông tin lên LCD 16x2.
  * Hiển thị icon trạng thái quạt (ON/OFF).

## 1. Cấu trúc thư mục (Folder Structure)

Dự án được tổ chức theo modules để dễ quản lý:

```
Core/
├── Inc/
│   ├── main.h
│   ├── FreeRTOSConfig.h
│   ├── app_tasks.h       // Khai báo các Task
│   ├── ds18b20.h         // Thư viện cảm biến
│   ├── liquidcrystal_i2c.h // Thư viện LCD
│   └── global_def.h      // Định nghĩa Struct và biến chung
├── Src/
│   ├── main.c            // Khởi tạo HAL, FreeRTOS Kernel
│   ├── app_tasks.c       // Logic chính của 4 Task
│   ├── ds18b20.c         // Driver cảm biến
│   ├── liquidcrystal_i2c.c // Driver LCD
│   └── stm32f1xx_it.c    // Ngắt (Interrupts)
```

## 2. Cấu hình CubeMX

1. **RCC:** HSE = Crystal/Ceramic Resonator (Clock 72MHz).
2. **SYS:** - Debug: Serial Wire.
   * **Timebase Source: TIM1** (Bắt buộc khi dùng FreeRTOS).
3. **FreeRTOS:** - Enable CMSIS_V1.
   * Tạo 4 Tasks và 1 Mutex (`SystemStateMutex`).
4. **GPIO:** Cấu hình như bảng Pin Mapping.

## 3. Ghi chú phát triển (Dev Notes)

* **Vấn đề Sensor:** DS18B20 ở độ phân giải 12-bit tốn 750ms để chuyển đổi. Để đáp ứng yêu cầu đọc 500ms, cần config cảm biến xuống **9-bit** hoặc **10-bit** trong driver.
* **Vấn đề LCD:** I2C hoạt động chậm, không nên gọi hàm LCD trong ngắt (ISR) hoặc các Task có độ ưu tiên quá cao (High Priority).

typedef struct {
    float currentTemp;      // Nhiệt độ hiện tại
    int8_t setTemp;         // Nhiệt độ cài đặt mong muốn
    uint8_t isFanOn;        // Trạng thái quạt (1: ON, 0: OFF)
    uint8_t mode;           // 0: OFF, 1: NORMAL, 2: SETTING
} ThermostatState_t;
