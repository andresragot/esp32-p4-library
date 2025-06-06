/**
 * @file driver_ek79007.cpp
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

#include "driver_ek79007.hpp"
#include "esp_lcd_ek79007.h"

namespace Ragot
{
    static const char * TAG = "DriverEK79007";

    static bool panel_refresh_callback (esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t * edata, void * user_ctx)
    {        
        DriverEK79007 * driver = static_cast < DriverEK79007 * > (user_ctx);
        
        if (driver == nullptr) 
        {
            return false;
        }
        
        if (driver->refresh_semaphore == nullptr) 
        {
            return false;
        }
        
        bool result = driver->refresh_frame_buffer(panel, edata, driver->refresh_semaphore);
                
        return result;
    }

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

        if (initialized)
        {
            ESP_LOGW(TAG, "Driver already initialized");
            return ESP_OK;
        }

        // Initialize the driver with the given parameters
        width = 1024;
        height = 600;
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

        // Encender la fuente de alimentación de MIPI DSI PHY
        esp_ldo_channel_handle_t ldo_mipi_phy = nullptr;
        esp_ldo_channel_config_t ldo_mipi_phy_config ={
            .chan_id = 3,
            .voltage_mv = 2500
        };
        ESP_ERROR_CHECK (esp_ldo_acquire_channel (&ldo_mipi_phy_config, &ldo_mipi_phy));
        ESP_LOGI (TAG, "MIPI DSI PHY Powered on");

        // Crear el bus MIPI DSI
        esp_lcd_dsi_bus_handle_t mipi_dsi_bus = nullptr;
        esp_lcd_dsi_bus_config_t bus_config = {
            .bus_id = 0,
            .num_data_lanes = mipi_lane_num,
            .lane_bit_rate_mbps = mipi_dsi_max_data_rate_mbps
        };
        ESP_ERROR_CHECK (esp_lcd_new_dsi_bus (&bus_config, &mipi_dsi_bus));

        // Configurar y crear el IO de control del panel
        esp_lcd_panel_io_handle_t mipi_dbi_io = nullptr;
        esp_lcd_dbi_io_config_t dbi_config = {
            .virtual_channel = 0,
            .lcd_cmd_bits   = 8, // according to the panel datasheet
            .lcd_param_bits = 8  // according to the panel datasheet
        };
        ESP_ERROR_CHECK (esp_lcd_new_panel_io_dbi (mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

        esp_lcd_dpi_panel_config_t panel_config = {
            .virtual_channel = 0,
            .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
            .dpi_clock_freq_mhz = panel_clk_freq_mhz,
            .pixel_format = pixel_format,
            .video_timing = {
                .h_size = width,
                .v_size = height,
                .hsync_pulse_width = hsync_pulse_width,
                .hsync_back_porch = hsync_back_porch,
                .hsync_front_porch = hsync_front_porch,
                .vsync_pulse_width = vsync_pulse_width,
                .vsync_back_porch = vsync_back_porch,
                .vsync_front_porch = vsync_front_porch
            },
            .flags = {
                .use_dma2d = true,
                .disable_lp = false
            }
        };

        ek79007_vendor_config_t vendor_config = {
            .mipi_config = {
                .dsi_bus = mipi_dsi_bus,
                .dpi_config = &panel_config,
            },
        };

        const esp_lcd_panel_dev_config_t lcd_dev_config = {
            .reset_gpio_num = reset_pin,
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
            .bits_per_pixel = 16,
            .vendor_config = &vendor_config,
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_ek79007(mipi_dbi_io, &lcd_dev_config, &handler));
    
        // Resetear e inicializar el panel
        ESP_ERROR_CHECK(esp_lcd_panel_reset(handler));
        ESP_ERROR_CHECK(esp_lcd_panel_init(handler));


        refresh_semaphore = xSemaphoreCreateBinary();
        if (refresh_semaphore == nullptr)
        {
            return ESP_ERR_NO_MEM;
        }

        esp_lcd_dpi_panel_event_callbacks_t cbs = {
            .on_refresh_done = panel_refresh_callback,
        };
        ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(handler, &cbs, this));

        bsp_enable_dsi_phy_power();
        bsp_init_lcd_backlight(bk_pin);

        initialized = true;
        ESP_LOGI(TAG, "Driver initialized successfully");

        return ESP_OK;
    }

    esp_err_t DriverEK79007::deinit()
    {
        if (handler)
        {
            esp_lcd_panel_del(handler);
            handler = nullptr;
        }

        if (refresh_semaphore)
        {
            vSemaphoreDelete(refresh_semaphore);
            refresh_semaphore = nullptr;
        }

        return ESP_OK;
    }

    esp_err_t DriverEK79007::send_frame_buffer( const void * frame_buffer)
    {
        ESP_LOGI(TAG, "send_frame_buffer() llamado con frame_buffer: %p", frame_buffer);
        esp_err_t ret = ESP_OK;

        if (handler)
        {
            if (refresh_semaphore == nullptr) {
                ESP_LOGE(TAG, "CRÍTICO: refresh_semaphore es NULL");
                return ESP_ERR_INVALID_STATE;
            }
            
            // Verificar estado del semáforo
            UBaseType_t sem_count = uxSemaphoreGetCount(refresh_semaphore);
            ESP_LOGI(TAG, "Estado del semáforo antes de tomar: %u", sem_count);
            
            ESP_LOGI(TAG, "Intentando tomar semáforo...");
            if (xSemaphoreTake(refresh_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE) {
                ESP_LOGE(TAG, "TIMEOUT esperando el semáforo");
                return ESP_ERR_TIMEOUT;
            }
            ESP_LOGI(TAG, "Semáforo tomado correctamente");
            
            ESP_LOGI(TAG, "Dibujando bitmap en panel");
            esp_err_t result = esp_lcd_panel_draw_bitmap(handler, 0, 0, width, height, frame_buffer);
            if (result != ESP_OK) 
            {
                ESP_LOGE(TAG, "Error en esp_lcd_panel_draw_bitmap: %s", esp_err_to_name(result));
                ret = result;
            } 
            else 
            {
                ESP_LOGI(TAG, "Bitmap dibujado correctamente");
            }
        }
        else
        {
            ESP_LOGE(TAG, "Panel handler is null");
            ret = ESP_ERR_INVALID_STATE;
        }

        return ret;
    }

    void DriverEK79007::bsp_enable_dsi_phy_power()
    {
        esp_ldo_channel_handle_t ldo_mipi_phy = nullptr;
        esp_ldo_channel_config_t ldo_mipi_phy_config ={
            .chan_id = 3,
            .voltage_mv = 2500
        };
        ESP_ERROR_CHECK (esp_ldo_acquire_channel (&ldo_mipi_phy_config, &ldo_mipi_phy));
        ESP_LOGI (TAG, "MIPI DSI PHY Powered on");
    }

    void DriverEK79007::bsp_init_lcd_backlight(gpio_num_t bk_pin)
    {
        gpio_config_t bk_gpio_config = { 0 };
        bk_gpio_config.mode = GPIO_MODE_OUTPUT;
        bk_gpio_config.pin_bit_mask = (1ULL << bk_pin);
        
        ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
        
        gpio_set_level(bk_pin, 1);
        ESP_LOGI(TAG, "Backlight powered on");
    }

    IRAM_ATTR bool DriverEK79007::refresh_frame_buffer(esp_lcd_panel_handle_t panel, 
                                                 esp_lcd_dpi_panel_event_data_t* edata, 
                                                 void* user_ctx)
    {        
        // Validar argumentos
        if (panel == nullptr) 
        {
            return false;
        }
        
        // Para usar el semáforo necesitamos castearlo desde el void*
        SemaphoreHandle_t refresh_sem = static_cast<SemaphoreHandle_t>(user_ctx);
        if (refresh_sem == nullptr) 
        {
            return false;
        }
        
        // Dar el semáforo
        BaseType_t result = xSemaphoreGive(refresh_sem);
        if (result != pdTRUE) 
        {
            return false;
        }

        return true;
    }

}