/**
 * @file driver_ek79007.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief 
 * @version 0.1
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "driver_ek79007.hpp"

namespace Ragot
{
    DriverEK79007::DriverEK79007()
    {
        init(GPIO_NUM_27, GPIO_NUM_26);
    }

    DriverEK79007::~DriverEK79007()
    {
        deinit();
    }

    esp_err_t DriverEK79007::init(gpio_num_t reset_pin, gpio_num_t bk_pin)
    {
        // Initialize the driver with the given parameters
        width = 600;
        height = 1024;
        pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565;

        panel_clk_freq_mhz = 52;
        hsync_pulse_width  = 20;
        hsync_back_porch   = 100;
        hsync_front_porch  = 100;
        vsync_pulse_width  = 3;
        vsync_back_porch   = 10;
        vsync_front_porch  = 10;
        mipi_lane_num      = 2;
        mipi_dsi_max_data_rate_mbps = 1000;

        esp_ldo_channel_handle_t ldo_mini_phy = nullptr;

        refresh_semaphore = xSemaphoreCreateBinary();
        if (refresh_semaphore == nullptr)
        {
            return ESP_ERR_NO_MEM;
        }

        return ESP_OK;
    }
}