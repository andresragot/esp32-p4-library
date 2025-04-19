/**
 * @file driver_led.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief 
 * @version 0.1
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_commands.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

namespace Ragot
{
    class DriverLED
    {
    public:
        DriverLED() = default;
        virtual ~DriverLED() = default;

        virtual esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) = 0;
        virtual esp_err_t deinit() = 0;
        virtual esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) = 0;
        virtual esp_err_t send_frame_buffer( const void * frame_buffer) = 0;

        const size_t  get_width() const { return width; }
              size_t  get_width()       { return width; }
        const size_t get_height() const { return height; }
              size_t get_height()       { return height; }

        const lcd_color_rgb_pixel_format_t get_pixel_format() const { return pixel_format; }
              lcd_color_rgb_pixel_format_t get_pixel_format()       { return pixel_format; }
        
        const esp_lcd_panel_handle_t get_handler() const { return handler; }
              esp_lcd_panel_handle_t get_handler()       { return handler; }

        const bool is_initialized() const { return initialized; }
              bool is_initialized()       { return initialized; }

    protected:
        bool initialized = false;
        size_t width;
        size_t height;
        lcd_color_rgb_pixel_format_t pixel_format;
        esp_lcd_panel_handle_t handler;
    };
}