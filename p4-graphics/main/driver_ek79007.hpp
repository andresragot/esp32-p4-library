/**
 * @file driver_ek79007.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the DriverEK79007 class, which manages the initialization and operation of the EK79007 LCD panel driver.
 * The DriverEK79007 class inherits from the DriverLCD class and provides methods to initialize, deinitialize, and send frame buffers to the LCD panel.
 * @version 1.0
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025 Andrés Ragot
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
 * 
 */

#pragma once

#include "driver_lcd.hpp"


namespace Ragot
{
    /**
     * @class DriverEK79007
     * @brief Driver for the EK79007 LCD panel.
     * 
     * This class provides methods to initialize, deinitialize, and send frame buffers to the EK79007 LCD panel.
     * It inherits from the DriverLCD class and implements the necessary methods for LCD operations.
     */
    class DriverEK79007 : public DriverLCD
    {
    public:

        /**
         * @brief Default constructor for the DriverEK79007 class
         */
        DriverEK79007();

        /**
         * @brief Destructor for the DriverEK79007 class
         * 
         * Cleans up resources used by the driver.
         */
       ~DriverEK79007() override;

        /**
         * @brief Initializes the EK79007 LCD panel driver.
         * 
         * This method sets up the panel with the specified reset and backlight GPIO pins,
         * configures the panel parameters, and registers the refresh callback.
         * 
         * @param reset_pin GPIO pin for panel reset.
         * @param bk_pin GPIO pin for backlight control.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) override;

        /**
         * @brief Deinitializes the EK79007 LCD panel driver.
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
         * This method is not implemented for the EK79007 driver and will return ESP_FAIL.
         * 
         * @param x X coordinate of the pixel.
         * @param y Y coordinate of the pixel.
         * @param color Color value to set the pixel to.
         * @return esp_err_t ESP_FAIL as this method is not implemented.
         */
        esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) override {return ESP_FAIL;};

        /**
         * @brief Sends a frame buffer to the EK79007 LCD panel.
         * 
         * This method sends the provided frame buffer to the panel for display.
         * It waits for the refresh semaphore to be available before sending the frame buffer.
         * 
         * @param frame_buffer Pointer to the frame buffer data.
         * @return esp_err_t ESP_OK on success, or an error code on failure.
         */
        esp_err_t send_frame_buffer(const void * frame_buffer) override;

        /**
         * @brief Refreshes the frame buffer for the EK79007 LCD panel.
         * 
         * This method is called when the panel refresh is done. It handles the actual drawing of the frame buffer
         * to the panel and releases the refresh semaphore.
         * 
         * @param panel Pointer to the LCD panel handle.
         * @param edata Pointer to the event data for the DPI panel.
         * @param user_ctx User context pointer, which is this driver instance.
         * @return true if successful, false otherwise.
         */
        IRAM_ATTR bool refresh_frame_buffer( esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t * edata, void * user_ctx);

    private:
        uint16_t panel_clk_freq_mhz; ///< Horizontal pixel clock frequency in MHz
        uint32_t hsync_pulse_width; ///< Horizontal sync width, in pixel clock
        uint32_t hsync_back_porch;  ///< Horizontal back porch, number of pixel clock between hsync and start of line active data
        uint32_t hsync_front_porch; ///< Horizontal front porch, number of pixel clock between the end of active data and the next hsync
        uint32_t vsync_pulse_width; ///< Vertical sync width, in number of lines
        uint32_t vsync_back_porch;  ///< Vertical back porch, number of invalid lines between vsync and start of frame
        uint32_t vsync_front_porch; ///< Vertical front porch, number of invalid lines between the end of frame and the next vsync
        uint8_t  mipi_lane_num;     ///< Number of MIPI DSI lanes used for the panel    
        uint16_t mipi_dsi_max_data_rate_mbps; ///< Maximum data rate of MIPI DSI in Mbps

    private:
        /**
         * @brief Enables the MIPI DSI PHY power.
         * 
         * This method powers on the MIPI DSI PHY by acquiring the appropriate LDO channel.
         */
        void bsp_enable_dsi_phy_power();

        /**
         * @brief Initializes the LCD backlight.
         * 
         * This method configures the GPIO pin for the backlight and sets it to high to turn on the backlight.
         * 
         * @param bk_pin GPIO pin number for the backlight control.
         */
        void bsp_init_lcd_backlight(gpio_num_t bk_pin);

    public:
        /**
         * @brief Refresh semaphore for synchronizing frame buffer updates.
         * 
         * This semaphore is used to ensure that the frame buffer is updated only when it is safe to do so.
         */
        SemaphoreHandle_t refresh_semaphore;
    };
}