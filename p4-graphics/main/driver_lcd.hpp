/**
 * @file driver_lcd.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file defines the DriverLCD class, which serves as a base class for LCD drivers in the Ragot engine.
 * @details The DriverLCD class provides an interface for initializing, deinitializing, setting pixels, and sending frame buffers to an LCD panel.
 * It includes methods to get the width, height, pixel format, and handler of the LCD panel.
 * The class is designed to be inherited by specific LCD driver implementations, such as the EK79007 driver.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
 * 
 * MIT License
 * 
 * Copyright (c) 2025 Andrés Ragot 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "esp_lcd_panel_ops.h"
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
#ifdef CONFIG_IDF_TARGET_ESP32P4
#include "esp_lcd_mipi_dsi.h"
#endif

namespace Ragot
{
      /**
       * @class DriverLCD
       * @brief Base class for LCD drivers.
       * 
       * This class provides an interface for LCD drivers, including methods for initialization, deinitialization,
       * setting pixels, and sending frame buffers. It also provides access to the LCD panel's width, height, pixel format,
       * and handler.
       */
      class DriverLCD
      {
      public:

            /**
             * @brief Default constructor for the DriverLCD class.
             * 
             * Initializes the LCD driver with default values.
             */
            DriverLCD() = default;

            /**
             * @brief Default virtual destructor for the DriverLCD class.
             * 
             * Cleans up resources used by the LCD driver.
             */
            virtual ~DriverLCD() = default;

            /**
             * @brief Initializes the LCD driver with the specified reset and backlight GPIO pins.
             * 
             * This method should be implemented by derived classes to set up the LCD panel.
             * 
             * @param reset_pin GPIO pin for panel reset.
             * @param bk_pin GPIO pin for backlight control.
             * @return esp_err_t ESP_OK on success, or an error code on failure.
             */
            virtual esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) = 0;

            /**
             * @brief Deinitializes the LCD driver.
             * 
             * This method should be implemented by derived classes to clean up resources used by the LCD panel.
             * 
             * @return esp_err_t ESP_OK on success, or an error code on failure.
             */
            virtual esp_err_t deinit() = 0;

            /**
             * @brief Sets a pixel at the specified coordinates to the given color.
             * 
             * This method should be implemented by derived classes to set a pixel on the LCD panel.
             * 
             * @param x X coordinate of the pixel.
             * @param y Y coordinate of the pixel.
             * @param color Color value for the pixel.
             * @return esp_err_t ESP_OK on success, or an error code on failure.
             */
            virtual esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) = 0;

            /**
             * @brief Sends a frame buffer to the LCD panel.
             * 
             * This method should be implemented by derived classes to send the provided frame buffer to the panel for display.
             * 
             * @param frame_buffer Pointer to the frame buffer data.
             * @return esp_err_t ESP_OK on success, or an error code on failure.
             */
            virtual esp_err_t send_frame_buffer( const void * frame_buffer) = 0;

            /**
             * @brief Gets the width of the LCD panel.
             * 
             * @return size_t Width of the LCD panel in pixels.
             */
            const size_t  get_width() const { return width; }

            /**
             * @brief Gets the height of the LCD panel.
             * 
             * @return size_t Height of the LCD panel in pixels.
             */
                  size_t  get_width()       { return width; }

            /**
             * @brief Gets the height of the LCD panel.
             * 
             * @return size_t Height of the LCD panel in pixels.
             */
            const size_t get_height() const { return height; }

            /**
             * @brief Gets the height of the LCD panel.
             * 
             * @return size_t Height of the LCD panel in pixels.
             */
                  size_t get_height()       { return height; }


            /**
             * @brief Get the pixel format object
             * 
             * @return const lcd_color_rgb_pixel_format_t 
             */
            const lcd_color_rgb_pixel_format_t get_pixel_format() const { return pixel_format; }

            /**
             * @brief Get the pixel format object
             * 
             * @return lcd_color_rgb_pixel_format_t 
             */
                  lcd_color_rgb_pixel_format_t get_pixel_format()       { return pixel_format; }
            
            /**
             * @brief Get the handler object
             * 
             * @return const esp_lcd_panel_handle_t 
             */
            const esp_lcd_panel_handle_t get_handler() const { return handler; }

            /**
             * @brief Get the handler object
             * 
             * @return esp_lcd_panel_handle_t 
             */
                  esp_lcd_panel_handle_t get_handler()       { return handler; }

            /**
             * @brief Checks if the LCD driver is initialized.
             * 
             * @return true 
             * @return false 
             */
            const bool is_initialized() const { return initialized; }

            /**
             * @brief Checks if the LCD driver is initialized.
             * 
             * @return true 
             * @return false 
             */
                  bool is_initialized()       { return initialized; }

      protected:
            bool initialized = false; ///< Flag indicating if the LCD driver is initialized.
            size_t width; ///< Width of the LCD panel in pixels.
            size_t height; ///< Height of the LCD panel in pixels.
            lcd_color_rgb_pixel_format_t pixel_format; ///< Pixel format of the LCD panel.
            esp_lcd_panel_handle_t handler; ///< Handle to the LCD panel.
    };
}