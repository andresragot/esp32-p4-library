/**
 * @file driver_ek79007.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief 
 * @version 0.1
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "driver_led.hpp"


namespace Ragot
{
    class DriverEK79007 : public DriverLED
    {
    public:
        DriverEK79007();
       ~DriverEK79007() override;

        esp_err_t init(gpio_num_t reset_pin, gpio_num_t bk_pin) override;
        esp_err_t deinit() override;
        virtual esp_err_t set_pixel(uint32_t x, uint32_t y, uint32_t color) = 0;
        esp_err_t send_frame_buffer( void * frame_buffer) override;

        IRAM_ATTR bool refresh_frame_buffer( esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t * edata, void * user_ctx)
        {
            SemaphoreHandle_t refresh_semaphore = static_cast < SemaphoreHandle_t > (user_ctx);
            BaseType_t need_yield = pdFALSE;

            xSemaphoreGiveFromISR(refresh_semaphore, &need_yield);
            
            return (need_yield == pdTRUE);
        }

    private:
        uint16_t panel_clk_freq_mhz;
        uint32_t hsync_pulse_width; /*!< Horizontal sync width, in pixel clock */
        uint32_t hsync_back_porch;  /*!< Horizontal back porch, number of pixel clock between hsync and start of line active data */
        uint32_t hsync_front_porch; /*!< Horizontal front porch, number of pixel clock between the end of active data and the next hsync */
        uint32_t vsync_pulse_width; /*!< Vertical sync width, in number of lines */
        uint32_t vsync_back_porch;  /*!< Vertical back porch, number of invalid lines between vsync and start of frame */
        uint32_t vsync_front_porch; /*!< Vertical front porch, number of invalid lines between the end of frame and the next vsync */
        uint8_t  mipi_lane_num;
        uint16_t mipi_dsi_max_data_rate_mbps;

    private:
        void bsp_enable_dsi_phy_power();
        void bsp_init_lcd_backlight(gpio_num_t bk_pin);

    public:
        SemaphoreHandle_t refresh_semaphore;
    };
}