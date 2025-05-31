/**
 * 
 */

#include "Driver_ST7789.hpp"
#include "Logger.hpp"
#include "driver/spi_common.h"
#include "esp_lcd_io_spi.h"


namespace Ragot
{
    static bool panel_refresh_callback (esp_lcd_panel_io_handle_t panel, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
    {        
        Driver_ST7789 * driver = static_cast < Driver_ST7789 * > (user_ctx);
        
        if (not driver) 
        {
            return false;
        }
        
        if (not driver->refresh_semaphore)
        {
            return false;
        }
        
        bool result = driver->refresh_frame_buffer(driver->refresh_semaphore);
                
        return result;
    }

    Driver_ST7789::Driver_ST7789 ()
    {
        init (GPIO_NUM_17, GPIO_NUM_1); // Default reset pin and no backlight pin
    }

    esp_err_t Driver_ST7789::init (gpio_num_t rest_pin, gpio_num_t bk_pin)
    {
        width = 320;
        height = 240;

        ESP_LOGI ( TAG, "Initializing LCD panel...");
        spi_bus_config_t buscfg = {
            .mosi_io_num = GPIO_NUM_16,
            .miso_io_num = -1,
            .sclk_io_num = GPIO_NUM_15,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000
        };

        esp_err_t ret = spi_bus_initialize (SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
        ESP_LOGD ( TAG, "spi_bus_initialize ret: %d", ret);

        esp_lcd_panel_io_handle_t io_handle = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {
            .cs_gpio_num = GPIO_NUM_8 ,
            .dc_gpio_num = GPIO_NUM_18,
            .spi_mode = 0,
            .pclk_hz = (80 * 1000 * 1000),
            .trans_queue_depth = 10,
            .on_color_trans_done = panel_refresh_callback,
            .user_ctx = this,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
        };

       ret = esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &io_handle);

        esp_lcd_panel_dev_config_t panel_config = {
            .reset_gpio_num = rest_pin,
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
            .bits_per_pixel = 16,
        };

        ESP_LOGD ( TAG, "esp_lcd_new_panel_io_spi ret: %d", ret);

        ret = esp_lcd_new_panel_st7789 (io_handle, &panel_config, &handler);
        if (ret != ESP_OK) 
        {
            ESP_LOGE (TAG, "esp_lcd_new_panel_st7789 ret: %d", ret);
            return false;
        }

        esp_lcd_panel_reset        (handler);
        esp_lcd_panel_init         (handler);
        esp_lcd_panel_swap_xy      (handler, true );
        esp_lcd_panel_disp_on_off  (handler, true );
        esp_lcd_panel_invert_color (handler, false);
        esp_lcd_panel_mirror       (handler, false, true);

        refresh_semaphore = xSemaphoreCreateBinary();
        if (refresh_semaphore == nullptr)
        {
            return ESP_ERR_NO_MEM;
        }

        initialized = true;
        xSemaphoreGive(refresh_semaphore);

        // esp_lcd_panel_dither_config_t dither = { .enable = false, /*…*/ };
        // esp_lcd_panel_dither_config(handler, &dither);

        ESP_LOGD ( TAG, "initializing LCD panel done");

        return true;
    }

    esp_err_t Driver_ST7789::deinit()
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
        return ESP_OK;
    }

    esp_err_t Driver_ST7789::send_frame_buffer( const void * frame_buffer)
    {
        ESP_LOGI(TAG, "send_frame_buffer() llamado con frame_buffer: %p", frame_buffer);
        esp_err_t ret = ESP_OK;

        if (handler)
        {
            if (refresh_semaphore == nullptr) 
            {
                ESP_LOGE(TAG, "CRÍTICO: refresh_semaphore es NULL");
                return ESP_ERR_INVALID_STATE;
            }
            
            // Verificar estado del semáforo
            UBaseType_t sem_count = uxSemaphoreGetCount(refresh_semaphore);
            ESP_LOGI(TAG, "Estado del semáforo antes de tomar: %u", sem_count);
            
            ESP_LOGI(TAG, "Intentando tomar semáforo...");
            if (xSemaphoreTake(refresh_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE) 
            {
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
    
    IRAM_ATTR bool Driver_ST7789::refresh_frame_buffer(void* user_ctx)
    {        
        // Para usar el semáforo necesitamos castearlo desde el void*
        SemaphoreHandle_t refresh_sem = static_cast<SemaphoreHandle_t>(user_ctx);
        if (not refresh_sem) 
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