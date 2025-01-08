
#include "esp_lcd_ek79007.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "EK79007 panel";

// Configuración del panel
#define PANEL_MIPI_DSI_DPI_CLK_MHZ  52
#define PANEL_MIPI_DSI_LCD_H_RES    1024
#define PANEL_MIPI_DSI_LCD_V_RES    600
#define PANEL_MIPI_DSI_LCD_HSYNC    20
#define PANEL_MIPI_DSI_LCD_HBP      100
#define PANEL_MIPI_DSI_LCD_HFP      100
#define PANEL_MIPI_DSI_LCD_VSYNC    3
#define PANEL_MIPI_DSI_LCD_VBP      10
#define PANEL_MIPI_DSI_LCD_VFP      10
#define PANEL_MIPI_DSI_LANE_NUM          2    // 2 data lanes
#define PANEL_MIPI_DSI_LANE_BITRATE_MBPS 1000 // 1Gbps

extern esp_lcd_panel_handle_t mipi_dpi_panel;
static SemaphoreHandle_t refresh_finish = NULL;

// esp_lcd_panel_handle_t ek79007_get_panel(void) {
//     return mipi_dpi_panel;
// }

void ek79007_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt) {
    if (h_res) {
        *h_res = PANEL_MIPI_DSI_LCD_H_RES;
    }
    if (v_res) {
        *v_res = PANEL_MIPI_DSI_LCD_V_RES;
    }
    if (color_fmt) {
        *color_fmt = LCD_COLOR_PIXEL_FORMAT_RGB565;
    }
}

void ek79007_initialize(gpio_num_t reset_pin) {
    // Encender la fuente de alimentación de MIPI DSI PHY
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = 3,
        .voltage_mv = 2500,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");

    // Crear el bus MIPI DSI
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = PANEL_MIPI_DSI_LANE_NUM,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = PANEL_MIPI_DSI_LANE_BITRATE_MBPS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    // Configurar y crear el IO de control del panel
    esp_lcd_panel_io_handle_t mipi_dbi_io;
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,   // according to the LCD spec
        .lcd_param_bits = 8, // according to the LCD spec
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    // Configurar y crear el panel MIPI DSI
    esp_lcd_dpi_panel_config_t dpi_config = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = PANEL_MIPI_DSI_DPI_CLK_MHZ,
        .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .video_timing = {
            .h_size = PANEL_MIPI_DSI_LCD_H_RES,
            .v_size = PANEL_MIPI_DSI_LCD_V_RES,
            .hsync_back_porch = PANEL_MIPI_DSI_LCD_HBP,
            .hsync_pulse_width = PANEL_MIPI_DSI_LCD_HSYNC,
            .hsync_front_porch = PANEL_MIPI_DSI_LCD_HFP,
            .vsync_back_porch = PANEL_MIPI_DSI_LCD_VBP,
            .vsync_pulse_width = PANEL_MIPI_DSI_LCD_VSYNC,
            .vsync_front_porch = PANEL_MIPI_DSI_LCD_VFP,
        },
        .flags.use_dma2d = true,
    };

    ek79007_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = reset_pin,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ek79007(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));

    // Resetear e inicializar el panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(mipi_dpi_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(mipi_dpi_panel));

    // Activar la retroiluminación y encender la pantalla
    // ek79007_backlight_on(mipi_dbi_io);
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(mipi_dpi_panel, true));

    // Registrar el evento de actualización del panel
    // refresh_finish = xSemaphoreCreateBinary();
    // if (refresh_finish == NULL) {
    //     ESP_LOGE(TAG, "Failed to create semaphore for refresh");
    //     return;
    // }
    
    // esp_lcd_dpi_panel_event_callbacks_t cbs = {
    //     .on_color_trans_done = test_notify_refresh_ready,
    // };
    // ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(mipi_dpi_panel, &cbs, refresh_finish));
}

