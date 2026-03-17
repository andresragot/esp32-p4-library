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
static constexpr int RGB_BITS_PER_PIXEL    = 16;
static constexpr int LCD_IO_RGB_HSYNC      = GPIO_NUM_46;
static constexpr int LCD_IO_RGB_VSYNC      = GPIO_NUM_3;
static constexpr int LCD_IO_RGB_DE         = GPIO_NUM_5;
static constexpr int LCD_IO_RGB_PCLK       = GPIO_NUM_7;
static constexpr int LCD_IO_RGB_DISP       = -1;
static constexpr int LCD_IO_RGB_DATA0      = GPIO_NUM_14;
static constexpr int LCD_IO_RGB_DATA1      = GPIO_NUM_38;
static constexpr int LCD_IO_RGB_DATA2      = GPIO_NUM_18;
static constexpr int LCD_IO_RGB_DATA3      = GPIO_NUM_17;
static constexpr int LCD_IO_RGB_DATA4      = GPIO_NUM_10;
static constexpr int LCD_IO_RGB_DATA5      = GPIO_NUM_39;
static constexpr int LCD_IO_RGB_DATA6      = GPIO_NUM_0;
static constexpr int LCD_IO_RGB_DATA7      = GPIO_NUM_45;
static constexpr int LCD_IO_RGB_DATA8      = GPIO_NUM_48;
static constexpr int LCD_IO_RGB_DATA9      = GPIO_NUM_47;
static constexpr int LCD_IO_RGB_DATA10     = GPIO_NUM_21;
static constexpr int LCD_IO_RGB_DATA11     = GPIO_NUM_1;
static constexpr int LCD_IO_RGB_DATA12     = GPIO_NUM_2;
static constexpr int LCD_IO_RGB_DATA13     = GPIO_NUM_42;
static constexpr int LCD_IO_RGB_DATA14     = GPIO_NUM_41;
static constexpr int LCD_IO_RGB_DATA15     = GPIO_NUM_40;

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

        esp_lcd_rgb_panel_config_t panel_config = {
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .timings = {
                .pclk_hz = LCD_PIXEL_CLOCK_HZ,
                .h_res = (uint32_t)width,
                .v_res = (uint32_t)height,
                .hsync_pulse_width = 162,
                .hsync_back_porch = 152,
                .hsync_front_porch = 48,
                .vsync_pulse_width = 45,
                .vsync_back_porch = 13,
                .vsync_front_porch = 3,
                .flags = {
                    .pclk_active_neg = 1,
                },
            },
            .data_width = RGB_DATA_WIDTH,
            .bits_per_pixel = RGB_BITS_PER_PIXEL,
            .num_fbs = 2,
            .bounce_buffer_size_px = 600 * 32,
            .sram_trans_align = 4,
            .psram_trans_align = 64,
            .hsync_gpio_num = LCD_IO_RGB_HSYNC,
            .vsync_gpio_num = LCD_IO_RGB_VSYNC,
            .de_gpio_num = LCD_IO_RGB_DE,
            .pclk_gpio_num = LCD_IO_RGB_PCLK,
            .disp_gpio_num = LCD_IO_RGB_DISP,
            .data_gpio_nums = {
                LCD_IO_RGB_DATA0,  LCD_IO_RGB_DATA1,  LCD_IO_RGB_DATA2,  LCD_IO_RGB_DATA3,
                LCD_IO_RGB_DATA4,  LCD_IO_RGB_DATA5,  LCD_IO_RGB_DATA6,  LCD_IO_RGB_DATA7,
                LCD_IO_RGB_DATA8,  LCD_IO_RGB_DATA9,  LCD_IO_RGB_DATA10, LCD_IO_RGB_DATA11,
                LCD_IO_RGB_DATA12, LCD_IO_RGB_DATA13, LCD_IO_RGB_DATA14, LCD_IO_RGB_DATA15,
            },
            .flags = {
                .fb_in_psram = 1,
            }
        };

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

    IRAM_ATTR bool Driver_ST7262::refresh_frame_buffer (void * user_ctx)
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
