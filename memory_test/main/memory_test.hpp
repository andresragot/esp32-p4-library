/**
 * @file memory_test.hpp
 * @author Andrés Ragot (andresragot99@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#pragma once

#include "esp_heap_caps.h"

 namespace Ragot
 {
    class MemoryTest
    {
    public:
        MemoryTest();
        ~MemoryTest();
        void printMemory(bool printAll = false);
        void printPointerDirection (void * pointer);

    private:
        static const char * TAG;
    };
 }
