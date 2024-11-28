
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_err.h"
#include "esp_log.h"
#include "dsi_panel_espressif_ek79007.h"
#include "esp_lcd_ek79007.h"
// #include "esp_lcd_types.h"
#include "esp_lcd_panel_interface.h"
#include "esp_timer.h"


static const char *TAG = "example";
#define PANEL_MIPI_DSI_PHY_PWR_LDO_CHAN             3 // LDO_VO3 is connected to VDD_MIPI_DPHY
#define PANEL_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV       2500
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL               1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL              !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT                    GPIO_NUM_26
#define EXAMPLE_PIN_NUM_LCD_RST                     GPIO_NUM_27

#define RED                                         0xF800
#define GREEN                                       0x07E0
#define BLUE                                        0x001F

esp_lcd_panel_handle_t mipi_dpi_panel = NULL;

static SemaphoreHandle_t refresh_finish = NULL;
static QueueHandle_t log_queue = NULL;

typedef struct {
    esp_lcd_panel_t * panel;
    uint16_t x;
    uint16_t y;
} pixel_task_params_t;

IRAM_ATTR static bool test_notify_refresh_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    SemaphoreHandle_t refresh_finish = (SemaphoreHandle_t)user_ctx;
    BaseType_t need_yield = pdFALSE;

    // Mensaje a enviar desde la ISR 
    // char *message = "Frame finished"; 
    // Envía el mensaje a la cola desde la ISR 
    // xQueueSendFromISR(log_queue, &message, &need_yield);

    xSemaphoreGiveFromISR(refresh_finish, &need_yield);

    return (need_yield == pdTRUE);
}

void log_task(void *pvParameter) 
{ 
    char* message; 
    
    while (true) 
    { 
        if (xQueueReceive(log_queue, &message, portMAX_DELAY)) 
        { 
            ESP_LOGI(TAG, "%s", message); 
        } 
    } 
}

void example_bsp_enable_dsi_phy_power(void) {
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    #ifdef PANEL_MIPI_DSI_PHY_PWR_LDO_CHAN
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = PANEL_MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = PANEL_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
    #endif
}

static void example_bsp_init_lcd_backlight(void) {
    #if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    #endif
}

static void example_bsp_set_lcd_backlight(uint32_t level) {
    #if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    // ESP_LOGE(TAG, "Setting level blacklight");
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, level);
    #endif
}

static void test_draw_color_bar(esp_lcd_panel_handle_t panel_handle, uint16_t h_res, uint16_t v_res)
{
    uint8_t byte_per_pixel = (24 + 7) / 8;
    uint16_t row_line = v_res / byte_per_pixel / 8;
    uint8_t *color = (uint8_t *)heap_caps_calloc(1, row_line * h_res * byte_per_pixel, MALLOC_CAP_DMA);

    for (int j = 0; j < byte_per_pixel * 8; j++) {
        for (int i = 0; i < row_line * h_res; i++) {
            for (int k = 0; k < byte_per_pixel; k++) {
                color[i * byte_per_pixel + k] = (BIT(j) >> (k * 8)) & 0xff;
            }
        }
        esp_lcd_panel_draw_bitmap(panel_handle, 0, j * row_line, h_res, (j + 1) * row_line, color);
        xSemaphoreTake(refresh_finish, portMAX_DELAY);
    }

    uint16_t color_line = row_line * byte_per_pixel * 8;
    uint16_t res_line = v_res - color_line;
    if (res_line) {
        for (int i = 0; i < res_line * h_res; i++) {
            for (int k = 0; k < byte_per_pixel; k++) {
                color[i * byte_per_pixel + k] = 0xff;
            }
        }
        esp_lcd_panel_draw_bitmap(panel_handle, 0, color_line, h_res, v_res, color);
        xSemaphoreTake(refresh_finish, portMAX_DELAY);
    }

    free(color);
}

static void test_draw_color_pattern (esp_lcd_panel_handle_t * panel_handle, uint16_t h_res, uint16_t v_res)
{
    // ESP_LOGI(TAG, "h_res: %i, v_res %i", h_res, v_res);
    size_t total_size = h_res * v_res * sizeof(uint16_t);
    size_t total_pixels = h_res * v_res;
    uint16_t *color = NULL;
    uint16_t *red_color     = (uint16_t*) heap_caps_malloc (total_size, MALLOC_CAP_SPIRAM);
    uint16_t *green_color   = (uint16_t*) heap_caps_malloc (total_size, MALLOC_CAP_SPIRAM);
    uint16_t *blue_color    = (uint16_t*) heap_caps_malloc (total_size, MALLOC_CAP_SPIRAM);
    uint64_t start_time, end_time;
    float fps, frame_time;

    if (red_color == NULL || green_color == NULL || blue_color == NULL)
    {
        ESP_LOGE(TAG, "RGB color not initialized!");
        return;
    }

    for (size_t i = 0; i < total_pixels; ++i)
    {
        red_color[i] = RED;
        green_color[i] = GREEN;
        blue_color[i] = BLUE;
    }

    esp_lcd_dpi_panel_get_frame_buffer(panel_handle, 1, &color);

    ESP_LOGI(TAG, "color size %zu", total_size);

    if (color == NULL)
    {
        ESP_LOGE(TAG, "Color not initialized");
        return;
    }

    ESP_LOGI(TAG, "Vamos a entrar en el bucle");

    while (true)
    {
        // Draw Red
        ESP_LOGI(TAG, "Drawing Red");
        // for (size_t i = 0; i < total_pixels; i++)
        // {
        //     color[i] = RED;

        //     if (i % 250 == 0)
        //         vTaskDelay(pdMS_TO_TICKS(10));
        // }
        // memset (color, RED, total_size); // Fill with red
        start_time = esp_timer_get_time();
        memcpy(color, red_color, total_size);

        esp_lcd_panel_draw_bitmap (panel_handle, 0, 0, h_res, v_res, color);
        while (xSemaphoreTake (refresh_finish, portMAX_DELAY) != pdTRUE)
        {
            ESP_LOGW(TAG, "Waiting Red");
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
        // vTaskDelay(pdMS_TO_TICKS(1));

        end_time = esp_timer_get_time();
        frame_time = (end_time - start_time) / 1000000.0f;
        fps = 1.0f / frame_time;
        ESP_LOGW(TAG, "FPS Red; %.2f", fps);


        ESP_LOGI(TAG, "Drawing Green");
        // for (size_t i = 0; i < total_pixels; i++)
        // {
        //     color[i] = GREEN;

        //     if (i % 250 == 0)
        //         vTaskDelay(pdMS_TO_TICKS(10));
        // }
        // memset (color, GREEN, total_size); // Fill with green
        start_time = esp_timer_get_time();
        memcpy(color, green_color, total_size);

        esp_lcd_panel_draw_bitmap (panel_handle, 0, 0, h_res, v_res, color);
        // xSemaphoreTake (refresh_finish, portMAX_DELAY);
        while (xSemaphoreTake (refresh_finish, portMAX_DELAY) != pdTRUE)
        {
            ESP_LOGW(TAG, "Waiting Green");
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
        // vTaskDelay(pdMS_TO_TICKS(1));

        end_time = esp_timer_get_time();
        frame_time = (end_time - start_time) / 1000000.0f;
        fps = 1.0f / frame_time;
        ESP_LOGW(TAG, "FPS Red; %.2f", fps);

        // esp_lcd_panel_draw_bitmap (panel_handle, 0, 300, h_res, 300 + 2, color);
        // xSemaphoreTake (refresh_finish, portMAX_DELAY);

        ESP_LOGI(TAG, "Drawing Blue");
        // for (size_t i = 0; i < total_pixels; i++)
        // {
        //     color[i] = BLUE;

        //     if (i % 250 == 0)
        //         vTaskDelay(pdMS_TO_TICKS(10));
        // }

        // memset (color, BLUE, total_size); // Fill with blue
        start_time = esp_timer_get_time();
        memcpy(color, blue_color, total_size);
        esp_lcd_panel_draw_bitmap (panel_handle, 0, 0, h_res, v_res, color);
        // xSemaphoreTake (refresh_finish, portMAX_DELAY);
        while (xSemaphoreTake (refresh_finish, portMAX_DELAY) != pdTRUE)
        {
            ESP_LOGW(TAG, "Waiting Blue");
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
        // vTaskDelay(pdMS_TO_TICKS(1));

        end_time = esp_timer_get_time();
        frame_time = (end_time - start_time) / 1000000.0f;
        fps = 1.0f / frame_time;
        ESP_LOGW(TAG, "FPS Red; %.2f", fps);

        // esp_lcd_panel_draw_bitmap (panel_handle, 0, 0, h_res, 300 + 2, color);
        // xSemaphoreTake (refresh_finish, portMAX_DELAY);

    }
    
    free(color);
}

// Función para imprimir el formato de color actual
void print_color_format(lcd_color_rgb_pixel_format_t color_format)
{
    switch (color_format)
    {
        case LCD_COLOR_PIXEL_FORMAT_RGB565:
            printf("Current color format is RGB565\n");
            break;
        case LCD_COLOR_PIXEL_FORMAT_RGB666:
            printf("Current color format is RGB666\n");
            break;
        case LCD_COLOR_PIXEL_FORMAT_RGB888:
            printf("Current color format is RGB888\n");
            break;
        default:
            printf("Unknown color format\n");
            break;
    }
}

void cycle_pixel_colors(void *pvParameters)
{
    pixel_task_params_t *params = (pixel_task_params_t *) pvParameters;
    uint16_t colors[] = {RED, GREEN, BLUE};
    uint8_t color_index = 0;

    ESP_LOGI(TAG, "Entrando en cycle pixel colors");
    ESP_LOGI(TAG, "valores x: %i, y: %i", params->x, params->y);

    while (true)
    {
        ESP_LOGI(TAG, "Setting pixel color to %d", colors[color_index]);
        // esp_err_t ret = panel_ek79007_set_pixel(params->panel, params->x, params->y, colors[color_index]);
        // if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Error setting pixel color");
        }

        // esp_lcd_panel_draw_bitmap(params->panel, params->x, params->y, params->x + 1, params->y + 1, colors[color_index]);

        color_index = (color_index + 1) % 3;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) 
{
    example_bsp_enable_dsi_phy_power();
    example_bsp_init_lcd_backlight();
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    size_t h_res = 0;
    size_t v_res = 0;
    lcd_color_rgb_pixel_format_t color_fmt = LCD_COLOR_PIXEL_FORMAT_RGB565;
    
    ek79007_initialize(EXAMPLE_PIN_NUM_LCD_RST);
    ek79007_get_parameters(&h_res, &v_res, &color_fmt);

    print_color_format(color_fmt);

    refresh_finish = xSemaphoreCreateBinary();

    log_queue = xQueueCreate(10, sizeof(char*));
    xTaskCreatePinnedToCore(log_task, "log_task", 2048, NULL, 5, NULL, 1);

    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_color_trans_done = test_notify_refresh_ready,
    };
    esp_lcd_dpi_panel_register_event_callbacks(mipi_dpi_panel, &cbs, refresh_finish);

    vTaskDelay(pdMS_TO_TICKS(10));

    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    
    // test_draw_color_bar(mipi_dpi_panel, h_res, v_res);
    ESP_LOGI(TAG, "h_res %zu, v_res %zu", h_res, v_res);
    // test_draw_color_pattern(mipi_dpi_panel, h_res, v_res);
    panel_ek79007_set_pixel (mipi_dpi_panel, h_res / 2, v_res / 2, RED);
    panel_ek79007_set_pixel (mipi_dpi_panel, 0, 0, RED);
    panel_ek79007_set_pixel (mipi_dpi_panel, h_res, v_res, RED);

    for (size_t i = 0; i < h_res; ++i)
    {
        for (size_t j = 0; j < v_res; ++j)
        {
            panel_ek79007_set_pixel(mipi_dpi_panel, i, j, RED);
        }
    }
}
