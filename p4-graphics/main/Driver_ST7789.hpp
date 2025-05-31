/**
 * @file Driver_Display.hpp
 * @brief Header file for the ST7789 display driver
 * This file defines the Driver_ST7789 class, which implements the Driver_Display interface
 * for controlling the ST7789 display.
 * 
 * @author Andrés Ragot
 * @date 2025-05-26
 */

#pragma once
#include "driver_lcd.hpp"

namespace Ragot
{

    class Driver_ST7789 : public DriverLCD
    {
    private:
        static constexpr const char * TAG = "[Driver_ST7789]...";

    public:
        Driver_ST7789 ();

        virtual ~Driver_ST7789() = default;

        esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) override;
        esp_err_t deinit() override;
        esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) override {return ESP_FAIL;};
        esp_err_t send_frame_buffer( const void * frame_buffer) override;

        IRAM_ATTR bool refresh_frame_buffer(void * user_ctx);

    private:
        void bsp_init_lcd_backlight(gpio_num_t bk_pin);

    public:
        SemaphoreHandle_t refresh_semaphore;
    };
}