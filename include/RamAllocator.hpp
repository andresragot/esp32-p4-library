/**
 * @file RamAllocator.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements a custom memory allocator for PSRAM in the Ragot engine.
 * @details The PSRAMAllocator class provides a way to allocate and deallocate memory in PSRAM with specific flags.
 * It is designed to be used with standard containers like std::vector, allowing for efficient memory management in embedded systems.
 * The allocator uses ESP-IDF's heap_caps_malloc and heap_caps_free functions to manage memory.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
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
 */

#pragma once
#include "esp_heap_caps.h"

namespace Ragot 
{

    /**
     * @class PSRAMAllocator
     * @brief Custom memory allocator for PSRAM.
     * 
     * This class provides a custom memory allocator that uses PSRAM with specific flags.
     * It can be used with standard containers like std::vector to manage memory in embedded systems.
     */
    template <typename T, uint16_t Flag>
    class PSRAMAllocator 
    {
    public:
        using value_type    = T;
        using pointer       = T*;
        using size_type     = std::size_t;

        /**
         * @struct rebind
         * @brief Rebinds the allocator to a different type.
         * This struct allows the PSRAMAllocator to be used with different types while maintaining the same allocation flags.
         * 
         * @tparam U 
         */
        template <typename U>
        struct rebind { using other = PSRAMAllocator<U, Flag>; };

        /**
         * @brief Default constructor for PSRAMAllocator.
         * 
         * This constructor initializes the PSRAMAllocator without any specific parameters.
         */
        PSRAMAllocator() noexcept {}

        /**
         * @brief Copy constructor for PSRAMAllocator.
         * 
         * This constructor allows copying of the PSRAMAllocator, but it does not perform any specific actions.
         * It is designed to be used with standard containers that require copyable allocators.
         * 
         * @tparam U The type to rebind to.
         * @param other The allocator to copy from.
         */
        template <typename U, uint16_t F2>
        PSRAMAllocator(const PSRAMAllocator<U, F2>&) noexcept {}

        /**
         * @brief Allocates memory for n objects of type T in PSRAM.
         * 
         * This method allocates memory for n objects of type T using heap_caps_malloc with the specified flags.
         * If the allocation fails, it throws std::bad_alloc.
         * 
         * @param n The number of objects to allocate memory for.
         * @return T* Pointer to the allocated memory.
         */
        T* allocate(size_type n) 
        {
            T* p = static_cast<T*>(heap_caps_malloc(n * sizeof(T), Flag));
            if (not p) throw std::bad_alloc();
            return p;
        }

        /**
         * @brief Deallocates memory for n objects of type T in PSRAM.
         * 
         * This method deallocates memory for n objects of type T using heap_caps_free.
         * It does not throw any exceptions.
         * 
         * @param p Pointer to the memory to deallocate.
         * @param n The number of objects to deallocate (not used).
         */
        void deallocate(T* p, size_type) noexcept 
        {
            heap_caps_free(p);
        }
    };
    
    /**
     * @brief Equality operator for PSRAMAllocator.
     * 
     * This operator checks if two PSRAMAllocators are equal based on their flags.
     * 
     * @tparam T The type of the first allocator.
     * @tparam F1 The flag of the first allocator.
     * @tparam U The type of the second allocator.
     * @tparam F2 The flag of the second allocator.
     * @param a The first PSRAMAllocator.
     * @param b The second PSRAMAllocator.
     * @return true if the flags are equal, false otherwise.
     */
    template <typename T, uint16_t F1, typename U, uint16_t F2>
    bool operator==(const PSRAMAllocator<T, F1>&, const PSRAMAllocator<U, F2>&) 
    { 
      return F1 == F2; 
    }

    /**
     * @brief Inequality operator for PSRAMAllocator.
     * 
     * This operator checks if two PSRAMAllocators are not equal based on their flags.
     * 
     * @tparam T The type of the first allocator.
     * @tparam F1 The flag of the first allocator.
     * @tparam U The type of the second allocator.
     * @tparam F2 The flag of the second allocator.
     * @param a The first PSRAMAllocator.
     * @param b The second PSRAMAllocator.
     * @return true if the flags are not equal, false otherwise.
     */
    template <typename T, uint16_t F1, typename U, uint16_t F2>
    bool operator!=(const PSRAMAllocator<T, F1>& a, const PSRAMAllocator<U, F2>& b) 
    {
      return !(a == b);
    }

} // namespace Ragot
