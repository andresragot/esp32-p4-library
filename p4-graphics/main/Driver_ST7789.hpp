/**
 * @file Driver_ST7789.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Driver_ST7789 class, which manages the initialization and operation of the ST7789 LCD panel driver.
 * The Driver_ST7789 class inherits from the DriverLCD class and provides methods to initialize, deinitialize, and send frame buffers to the LCD panel.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
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
#include "driver_lcd.hpp"

namespace Ragot
{
    /**
     * @class Driver_ST7789
     * @brief Driver for the ST7789 LCD panel.
     * 
     * This class provides methods to initialize, deinitialize, and send frame buffers to the ST7789 LCD panel.
     * It inherits from the DriverLCD class and implements the necessary methods for LCD operations.
     */
    class Driver_ST7789 : public DriverLCD
    {
    private:
        /**
         * @brief Tag for logging messages related to the ST7789 driver.
         */
        static constexpr const char * TAG = "[Driver_ST7789]...";

    public:
        /**
         * @brief Default constructor for the Driver_ST7789 class.
         * 
         * Initializes the LCD driver with default values.
         */
        Driver_ST7789 ();

        /**
         * @brief Default virtual destructor for the Driver_ST7789 class.
         * 
         * Cleans up resources used by the LCD driver.
         */
        virtual ~Driver_ST7789() = default;

        /**
         * @brief Initializes the ST7789 LCD panel driver.
         * 
         * This method sets up the LCD panel with the specified reset and backlight GPIO pins.
         * It configures the panel's pixel format, width, height, and other parameters.
         * 
         * @param reset_pin GPIO pin for panel reset.
         * @param bk_pin GPIO pin for backlight control.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) override;

        /**
         * @brief Deinitializes the ST7789 LCD panel driver.
         * 
         * This method cleans up resources used by the driver, including deleting the panel handler
         * and freeing the refresh semaphore.
         * 
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t deinit() override;

        /**
         * @brief Sets a pixel at the specified coordinates to the given color.
         * 
         * This method is not implemented for the ST7789 driver and will return ESP_FAIL.
         * 
         * @param x X coordinate of the pixel.
         * @param y Y coordinate of the pixel.
         * @param color Color value for the pixel.
         * @return esp_err_t ESP_FAIL as this method is not implemented.
         */
        esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) override {return ESP_FAIL;};

        /**
         * @brief Sends a frame buffer to the ST7789 LCD panel.
         * 
         * This method sends the provided frame buffer to the panel for display.
         * It waits for the refresh semaphore to be available before sending the frame buffer.
         * 
         * @param frame_buffer Pointer to the frame buffer data.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t send_frame_buffer( const void * frame_buffer) override;

        /**
         * @brief Refreshes the frame buffer for the ST7789 LCD panel.
         * 
         * @note This method is called from the ISR context and should be used to signal that the frame buffer has been updated.
         * 
         * @param user_ctx 
         * @return bool True if the frame buffer was refreshed successfully, false otherwise.
         */
        IRAM_ATTR bool refresh_frame_buffer(void * user_ctx);

    private:
        /**
         * @brief Initializes the LCD backlight GPIO pin.
         * 
         * This method sets up the specified GPIO pin for controlling the LCD backlight.
         * 
         * @param bk_pin GPIO pin for backlight control.
         */
        void bsp_init_lcd_backlight(gpio_num_t bk_pin);

    public:
        /**
         * @brief Refresh semaphore for synchronizing frame buffer updates.
         */
        SemaphoreHandle_t refresh_semaphore;
    };
}