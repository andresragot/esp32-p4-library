#pragma once

#include "esp_lcd_mipi_dsi.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_commands.h"

typedef struct esp_lcd_panel_t esp_lcd_panel_t;

esp_lcd_panel_handle_t ek79007_get_panel(void);
void ek79007_initialize(gpio_num_t reset_pin);
void ek79007_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt);
esp_err_t panel_ek79007_set_pixel (esp_lcd_panel_handle_t *panel, uint16_t x, uint16_t y, uint16_t color);
