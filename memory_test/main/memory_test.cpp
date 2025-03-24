/**
 * @file memory_test.cpp
 * @author Andrés Ragot (andresragot99@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include "memory_test.hpp"
#include "esp_log.h"

namespace Ragot
{
    const char * MemoryTest::TAG = "MemoryTest";

    MemoryTest::MemoryTest()
    {
        esp_log_level_set(TAG, ESP_LOG_INFO);
        ESP_LOGI(TAG, "MemoryTest constructor");
    }

    MemoryTest::~MemoryTest()
    {
        ESP_LOGW(TAG, "MemoryTest destructor");
    }

    void MemoryTest::printMemory(bool printAll)
    {
        if (printAll)
            heap_caps_print_heap_info(MALLOC_CAP_8BIT);

        ESP_LOGI(TAG, "Free heap size: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    }

    void MemoryTest::printPointerDirection (void * pointer)
    {
        ESP_LOGI(TAG, "Pointer direction: %p", pointer);
    }
}
