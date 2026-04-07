#include "pxt.h"
#include "MicroBit.h"
#include "FastUARTService.h"

using namespace pxt;

namespace fastble {
#if MICROBIT_CODAL
    // 修正點：使用完整的命名空間路徑
    codal::FastUARTService *uart = nullptr;
#endif
    
    uint8_t sensor_addr = 0; 
    bool is_qma = false;
    uint8_t scale_gs[] = {2, 4, 8, 16};
    int current_scale_idx = 2; 
    bool display_enabled = true;
    bool capturing = false;

    // 取得 micro:bit 的 5 字元隨機名稱 (例如 zoged)
    // 這在 V1/V2 都是唯一的，可以方便辨識機器
    //%
    void showFriendlyName() {
        // 使用 microbit_friendly_name() 取得名稱
        ManagedString name = microbit_friendly_name();
        uBit.display.scroll(name);
    }

    void update_sensor_scale() {
        if (sensor_addr == 0x32) {
            uint8_t reg23[] = {0x23, (uint8_t)(0x80 | (current_scale_idx << 4))}; 
#if MICROBIT_CODAL
            uBit.i2c.write(sensor_addr, reg23, 2);
#else
            uBit.i2c.write(sensor_addr, (const char*)reg23, 2);
#endif
        } else if (sensor_addr == 0x24 || sensor_addr == 0x26) {
            uint8_t range_val = (current_scale_idx == 0) ? 0x01 : (current_scale_idx == 1) ? 0x02 : (current_scale_idx == 2) ? 0x04 : 0x08;
            uint8_t range[] = {0x0F, range_val}; 
#if MICROBIT_CODAL
            uBit.i2c.write(sensor_addr, range, 2);
#else
            uBit.i2c.write(sensor_addr, (const char*)range, 2);
#endif
        }
        
        if (display_enabled) {
            for(int i=0; i<5; i++) uBit.display.image.setPixelValue(i, 0, (i <= current_scale_idx) ? 255 : 0);
        }
    }

    void sampling_fiber() {
        uint32_t global_sample_index = 0;
        uint8_t packet[128]; 
        int packed_samples = 0;
        int dot_x = 0;
        bool last_btn_a = false;
        bool last_btn_b = false;

        while(capturing) {
            uint64_t start_loop = system_timer_current_time();

            if (global_sample_index % 100 == 0) {
                bool btn_a = uBit.buttonA.isPressed();
                if (btn_a && !last_btn_a) {
                    current_scale_idx = (current_scale_idx + 1) % 4;
                    update_sensor_scale();
                }
                last_btn_a = btn_a;

                bool btn_b = uBit.buttonB.isPressed();
                if (btn_b && !last_btn_b) {
                    display_enabled = !display_enabled;
                    if (!display_enabled) uBit.display.clear();
                    else update_sensor_scale();
                }
                last_btn_b = btn_b;
            }

            uint8_t accel_raw[6] = {0,0,0,0,0,0};
            if (sensor_addr != 0) {
                uint8_t cmd = is_qma ? 0x01 : 0xA8;
#if MICROBIT_CODAL
                uBit.i2c.write(sensor_addr, &cmd, 1, true);
                uBit.i2c.read(sensor_addr, accel_raw, 6);
#else
                uBit.i2c.write(sensor_addr, (const char*)&cmd, 1, true);
                uBit.i2c.read(sensor_addr, (char*)accel_raw, 6);
#endif
            }

            int offset = packed_samples * 6;
            packet[offset] = accel_raw[1]; packet[offset+1] = accel_raw[0];
            packet[offset+2] = accel_raw[3]; packet[offset+3] = accel_raw[2];
            packet[offset+4] = accel_raw[5]; packet[offset+5] = accel_raw[4];

            if (++packed_samples >= 20) {
                packet[124] = (global_sample_index >> 24) & 0xFF;
                packet[125] = (global_sample_index >> 16) & 0xFF;
                packet[126] = (global_sample_index >> 8) & 0xFF;
                packet[127] = global_sample_index & 0xFF;
                
#if MICROBIT_CODAL
                // 這裡也需要確保 uart 指針是正確的
                if (uBit.ble && uart) {
                    uart->send(packet, 128, ASYNC);
                }
#endif
                packed_samples = 0;

                if (display_enabled) {
                    uBit.display.image.setPixelValue(dot_x, 4, 0);
                    dot_x = (dot_x + 1) % 5;
                    uBit.display.image.setPixelValue(dot_x, 4, 255);
                }
            }

            global_sample_index++;
            while ((system_timer_current_time() - start_loop) < 1); 
        }
    }

    //% shim=fastble::startCaptureNative
    void startCaptureNative() {
        if (capturing) return;

        // 啟動時顯示一次名稱，方便辨識
        showFriendlyName();

        uint8_t scan_reg = 0x00;
#if MICROBIT_CODAL
        if (uBit.i2c.write(0x32, &scan_reg, 1, true) == 0) sensor_addr = 0x32;
        else if (uBit.i2c.write(0x24, &scan_reg, 1, true) == 0) sensor_addr = 0x24;
#else
        if (uBit.i2c.write(0x32, (const char*)&scan_reg, 1, true) == 0) sensor_addr = 0x32;
        else if (uBit.i2c.write(0x24, (const char*)&scan_reg, 1, true) == 0) sensor_addr = 0x24;
#endif

        if (sensor_addr == 0x24) is_qma = true;

        if (!is_qma) {
            uint8_t reg20[] = {0x20, 0x97}; 
#if MICROBIT_CODAL
            uBit.i2c.write(sensor_addr, reg20, 2);
#else
            uBit.i2c.write(sensor_addr, (const char*)reg20, 2);
#endif
        } else {
            uint8_t reset[] = {0x11, 0x80}; 
#if MICROBIT_CODAL
            uBit.i2c.write(sensor_addr, reset, 2);
#else
            uBit.i2c.write(sensor_addr, (const char*)reset, 2);
#endif
            fiber_sleep(20);
            uint8_t mode[] = {0x11, 0x01}; 
#if MICROBIT_CODAL
            uBit.i2c.write(sensor_addr, mode, 2);
#else
            uBit.i2c.write(sensor_addr, (const char*)mode, 2);
#endif
        }
        update_sensor_scale();

#if MICROBIT_CODAL
        if (uart == nullptr && uBit.ble) {
            // 修正點：new 對象時也指定 codal 命名空間
            uart = new codal::FastUARTService(*uBit.ble, 2048, 2048);
        }
#endif

        capturing = true;
        create_fiber(sampling_fiber);
    }
}