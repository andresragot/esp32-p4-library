#include "Renderer.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* MAIN_TAG = "Main";

extern "C" void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE);

    ESP_LOGI(MAIN_TAG, "Iniciando aplicación");
    ESP_LOGI(MAIN_TAG, "Memoria libre inicial: %u bytes", esp_get_free_heap_size());
    
    ESP_LOGI(MAIN_TAG, "Creando renderer (600x1024)");
    Ragot::Renderer renderer(1024, 600);
    
    uint32_t frame_count = 0;
    ESP_LOGI(MAIN_TAG, "Entrando en bucle de renderizado");

    while (true)
    {
        ESP_LOGI(MAIN_TAG, "--- Frame %u ---", frame_count);
        renderer.render_debug();
        ESP_LOGI(MAIN_TAG, "Esperando 50ms");
        vTaskDelay(pdMS_TO_TICKS(50));
        
        frame_count++;
        
        // Log periódico del estado de memoria
        if (frame_count % 100 == 0) {
            ESP_LOGI(MAIN_TAG, "Estado después de %u frames:", frame_count);
            ESP_LOGI(MAIN_TAG, "Memoria libre: %u bytes", esp_get_free_heap_size());
        }
    }
}
