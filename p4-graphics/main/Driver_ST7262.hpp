/**
 * @file Driver_ST7262.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Driver for the ST7262 RGB LCD panel (1024x600, 16-bit parallel RGB).
 * @version 0.1
 * @date 2025-09-03
 * 
 * @copyright Copyright (c) 2025
 * MIT License
 */

#pragma once

#include "driver_lcd.hpp"

namespace Ragot
{
    /**
     * @class Driver_ST7262
     * @brief Driver for the ST7262 RGB LCD panel.
     * 
     * This class provides methods to initialize, deinitialize, and send frame buffers to the ST7262 LCD panel.
     * It inherits from the DriverLCD class and implements the necessary methods for LCD operations.
     */
    class Driver_ST7262 : public DriverLCD
    {
    private:
        static constexpr const char * TAG = "[Driver_ST7262]...";

    public:
        /**
         * @brief Default constructor for the Driver_ST7262 class.
         */
        Driver_ST7262 ();

        /**
         * @brief Default virtual destructor for the Driver_ST7262 class.
         */
        virtual ~Driver_ST7262 () = default;

        /**
         * @brief Initializes the ST7262 RGB LCD panel driver.
         * 
         * @param reset_pin GPIO pin for panel reset (unused for RGB panels).
         * @param bk_pin GPIO pin for backlight control.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t init (gpio_num_t reset_pin, gpio_num_t bk_pin) override;

        /**
         * @brief Deinitializes the ST7262 LCD panel driver.
         * 
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t deinit () override;

        /**
         * @brief Sets a pixel at the specified coordinates to the given color.
         * 
         * Not implemented for the ST7262 driver.
         * 
         * @param x X coordinate of the pixel.
         * @param y Y coordinate of the pixel.
         * @param color Color value for the pixel.
         * @return esp_err_t ESP_FAIL as this method is not implemented.
         */
        esp_err_t set_pixel (uint32_t x, uint32_t y, uint32_t color) override { return ESP_FAIL; }

        /**
         * @brief Sends a frame buffer to the ST7262 LCD panel.
         * 
         * @param frame_buffer Pointer to the frame buffer data.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t send_frame_buffer (const void * frame_buffer) override;

        /**
         * @brief Refreshes the frame buffer for the ST7262 LCD panel.
         * 
         * @param user_ctx User context (refresh semaphore).
         * @return true if the frame buffer was refreshed successfully, false otherwise.
         */
        IRAM_ATTR bool refresh_frame_buffer (void * user_ctx);

    public:
        /**
         * @brief Refresh semaphore for synchronizing frame buffer updates.
         */
        SemaphoreHandle_t refresh_semaphore;
    };
}
