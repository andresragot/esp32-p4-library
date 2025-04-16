#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "memory_test.hpp"
#include <memory>
#include <cassert>
#include "esp_log.h"
#include <vector>
#include "RamAllocator.hpp"

using namespace Ragot;

static const char * TAG = "P4";

extern "C" void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    std::unique_ptr < MemoryTest > memoryTest = std::make_unique < MemoryTest > ();
    ESP_LOGW (TAG, "MemoryTest created");
    memoryTest->printMemory();

    ESP_LOGW (TAG, "Memoery of memorytest");
    memoryTest->printPointerDirection(memoryTest.get());

    ESP_LOGW (TAG, "Allocating memory");
    void * pointer = malloc(10);
    assert(pointer);

    ESP_LOGW (TAG, "Memory allocated");
    memoryTest->printPointerDirection(pointer);
    memoryTest->printMemory();

    ESP_LOGW (TAG, "Freeing memory");
    free(pointer);
    memoryTest->printMemory();

    ESP_LOGW (TAG, "Allocating memory with new");
    pointer = new char[10];
    assert(pointer);

    ESP_LOGW (TAG, "Memory allocated");
    memoryTest->printPointerDirection(pointer);
    memoryTest->printMemory();

    ESP_LOGW (TAG, "Freeing memory with delete");
    delete[] pointer;
    memoryTest->printMemory();

    std::vector<char, PSRAMAllocator<char>> vector(10);
    ESP_LOGI(TAG, "Vector created in PSRAM");
    uint32_t i = 0;

    heaps_caps_malloc (232, MALLOC_CAP_8_BIT);

    void * pointer_to_vector = vector.data();
    while (true)
    {
        if (pointer_to_vector != vector.data() || i == 0)
        {
            pointer_to_vector = &vector;
            ESP_LOGE (TAG, "Pointer to vector changed");
            memoryTest->printMemory();
            memoryTest->printPointerDirection(&vector);
        }
        ++i;
        if (i % 1024 == 0)
            ESP_LOGI (TAG, "Iteration: %d", i);
        
            vector.emplace_back('a');
        vTaskDelay (pdMS_TO_TICKS(1));
    }
    
}
