/**
 * @file Driver_ST7262.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Driver for the ST7262 RGB LCD panel (1024x600, 16-bit parallel RGB).
 * @version 0.1
 * @date 2025-09-03
 * 
 * @copyright Copyright (c) 2025
 * MIT License
 */

#include "Driver_ST7262.hpp"
#include "Logger.hpp"
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <algorithm>

static bool panel_vsync_callback (esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t * edata, void * user_ctx)
{
    Ragot::Driver_ST7262 * driver = static_cast<Ragot::Driver_ST7262 *>(user_ctx);

    if (!driver || !driver->refresh_semaphore)
    {
        return false;
    }

    return driver->refresh_frame_buffer (driver->refresh_semaphore);
}

static constexpr int LCD_PIXEL_CLOCK_HZ    = 30.85 * 1000 * 1000;
static constexpr int RGB_DATA_WIDTH        = 16;
static constexpr lcd_color_format_t RGB_BITS_PER_PIXEL    = LCD_COLOR_FMT_RGB565;
static constexpr gpio_num_t LCD_IO_RGB_HSYNC      = GPIO_NUM_46;
static constexpr gpio_num_t LCD_IO_RGB_VSYNC      = GPIO_NUM_3;
static constexpr gpio_num_t LCD_IO_RGB_DE         = GPIO_NUM_5;
static constexpr gpio_num_t LCD_IO_RGB_PCLK       = GPIO_NUM_7;
static constexpr gpio_num_t LCD_IO_RGB_DISP       = GPIO_NUM_NC;
static constexpr gpio_num_t LCD_IO_RGB_DATA0      = GPIO_NUM_14;
static constexpr gpio_num_t LCD_IO_RGB_DATA1      = GPIO_NUM_38;
static constexpr gpio_num_t LCD_IO_RGB_DATA2      = GPIO_NUM_18;
static constexpr gpio_num_t LCD_IO_RGB_DATA3      = GPIO_NUM_17;
static constexpr gpio_num_t LCD_IO_RGB_DATA4      = GPIO_NUM_10;
static constexpr gpio_num_t LCD_IO_RGB_DATA5      = GPIO_NUM_39;
static constexpr gpio_num_t LCD_IO_RGB_DATA6      = GPIO_NUM_0;
static constexpr gpio_num_t LCD_IO_RGB_DATA7      = GPIO_NUM_45;
static constexpr gpio_num_t LCD_IO_RGB_DATA8      = GPIO_NUM_48;
static constexpr gpio_num_t LCD_IO_RGB_DATA9      = GPIO_NUM_47;
static constexpr gpio_num_t LCD_IO_RGB_DATA10     = GPIO_NUM_21;
static constexpr gpio_num_t LCD_IO_RGB_DATA11     = GPIO_NUM_1;
static constexpr gpio_num_t LCD_IO_RGB_DATA12     = GPIO_NUM_2;
static constexpr gpio_num_t LCD_IO_RGB_DATA13     = GPIO_NUM_42;
static constexpr gpio_num_t LCD_IO_RGB_DATA14     = GPIO_NUM_41;
static constexpr gpio_num_t LCD_IO_RGB_DATA15     = GPIO_NUM_40;

namespace Ragot
{
    Driver_ST7262::Driver_ST7262 ()
    {
        init (GPIO_NUM_NC, GPIO_NUM_NC);
    }

    esp_err_t Driver_ST7262::init (gpio_num_t reset_pin, gpio_num_t bk_pin)
    {
        width  = 1024;
        height = 600;

        ESP_LOGI (TAG, "Installing RGB LCD panel driver");
        handler = nullptr;

        esp_lcd_rgb_panel_config_t panel_config = {};
        panel_config.clk_src = LCD_CLK_SRC_DEFAULT;
        
        panel_config.timings = {};
        panel_config.timings.pclk_hz = LCD_PIXEL_CLOCK_HZ;
        panel_config.timings.h_res = (uint32_t)width;
        panel_config.timings.v_res = (uint32_t)height;
        panel_config.timings.hsync_pulse_width = 162;
        panel_config.timings.hsync_back_porch = 152;
        panel_config.timings.hsync_front_porch = 48;
        panel_config.timings.vsync_pulse_width = 45;
        panel_config.timings.vsync_back_porch = 13;
        panel_config.timings.vsync_front_porch = 3;
        
        panel_config.timings.flags = {};
        panel_config.timings.flags.pclk_active_neg = 1;
        
        panel_config.data_width = RGB_DATA_WIDTH;
        panel_config.in_color_format = RGB_BITS_PER_PIXEL;
        panel_config.out_color_format = RGB_BITS_PER_PIXEL;
        panel_config.num_fbs = 2;
        panel_config.bounce_buffer_size_px = 600 * 32;
        panel_config.dma_burst_size = 4;
        panel_config.hsync_gpio_num = LCD_IO_RGB_HSYNC;
        panel_config.vsync_gpio_num = LCD_IO_RGB_VSYNC;
        panel_config.de_gpio_num = LCD_IO_RGB_DE;
        panel_config.pclk_gpio_num = LCD_IO_RGB_PCLK;
        panel_config.disp_gpio_num = LCD_IO_RGB_DISP;
        
        panel_config.data_gpio_nums[0]  = LCD_IO_RGB_DATA0;
        panel_config.data_gpio_nums[1]  = LCD_IO_RGB_DATA1;
        panel_config.data_gpio_nums[2]  = LCD_IO_RGB_DATA2;
        panel_config.data_gpio_nums[3]  = LCD_IO_RGB_DATA3;
        panel_config.data_gpio_nums[4]  = LCD_IO_RGB_DATA4;
        panel_config.data_gpio_nums[5]  = LCD_IO_RGB_DATA5;
        panel_config.data_gpio_nums[6]  = LCD_IO_RGB_DATA6;
        panel_config.data_gpio_nums[7]  = LCD_IO_RGB_DATA7;
        panel_config.data_gpio_nums[8]  = LCD_IO_RGB_DATA8;
        panel_config.data_gpio_nums[9]  = LCD_IO_RGB_DATA9;
        panel_config.data_gpio_nums[10] = LCD_IO_RGB_DATA10;
        panel_config.data_gpio_nums[11] = LCD_IO_RGB_DATA11;
        panel_config.data_gpio_nums[12] = LCD_IO_RGB_DATA12;
        panel_config.data_gpio_nums[13] = LCD_IO_RGB_DATA13;
        panel_config.data_gpio_nums[14] = LCD_IO_RGB_DATA14;
        panel_config.data_gpio_nums[15] = LCD_IO_RGB_DATA15;
        
        panel_config.flags = {};
        panel_config.flags.fb_in_psram = 1;

        esp_err_t err = esp_lcd_new_rgb_panel (&panel_config, &handler);
        if (err != ESP_OK) 
        {
            ESP_LOGE (TAG, "esp_lcd_new_rgb_panel failed: %s (0x%x)", esp_err_to_name (err), err);
            return err;
        }

        ESP_LOGI (TAG, "Initialize RGB LCD panel");
        err = esp_lcd_panel_init (handler);
        if (err != ESP_OK)
        {
            ESP_LOGE (TAG, "esp_lcd_panel_init failed: %s (0x%x)", esp_err_to_name (err), err);
            return err;
        }

        refresh_semaphore = xSemaphoreCreateBinary ();
        if (refresh_semaphore == nullptr)
        {
            return ESP_ERR_NO_MEM;
        }
        xSemaphoreGive (refresh_semaphore);

        esp_lcd_rgb_panel_event_callbacks_t cbs = {};
        cbs.on_vsync = panel_vsync_callback;
        err = esp_lcd_rgb_panel_register_event_callbacks (handler, &cbs, this);
        if (err != ESP_OK)
        {
            ESP_LOGE (TAG, "Failed to register vsync callback: %s", esp_err_to_name (err));
            return err;
        }

        initialized = true;
        ESP_LOGI (TAG, "Driver_ST7262 initialized (%zux%zu)", width, height);
        return ESP_OK;
    }

    esp_err_t Driver_ST7262::deinit ()
    {
        if (handler)
        {
            esp_lcd_panel_del (handler);
            handler = nullptr;
        }

        if (refresh_semaphore)
        {
            vSemaphoreDelete (refresh_semaphore);
            refresh_semaphore = nullptr;
        }

        initialized = false;
        return ESP_OK;
    }

    esp_err_t Driver_ST7262::send_frame_buffer (const void * frame_buffer)
    {
        if (!handler)
        {
            ESP_LOGE (TAG, "Panel handler is null");
            return ESP_ERR_INVALID_STATE;
        }

        if (refresh_semaphore == nullptr)
        {
            ESP_LOGE (TAG, "refresh_semaphore is NULL");
            return ESP_ERR_INVALID_STATE;
        }

        if (xSemaphoreTake (refresh_semaphore, pdMS_TO_TICKS (1000)) != pdTRUE)
        {
            ESP_LOGE (TAG, "Timeout waiting for semaphore");
            return ESP_ERR_TIMEOUT;
        }

        esp_err_t ret = esp_lcd_panel_draw_bitmap (handler, 0, 0, width, height, frame_buffer);
        if (ret != ESP_OK)
        {
            ESP_LOGE (TAG, "esp_lcd_panel_draw_bitmap error: %s", esp_err_to_name (ret));
        }

        return ret;
    }

    bool Driver_ST7262::refresh_frame_buffer (void * user_ctx)
    {
        SemaphoreHandle_t refresh_sem = static_cast<SemaphoreHandle_t> (user_ctx);
        if (!refresh_sem)
        {
            return false;
        }

        BaseType_t result = xSemaphoreGive (refresh_sem);
        return result == pdTRUE;
    }
}
