/**
 * @file atomic_stubs.c
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file provides atomic operations using GCC built-in functions.
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


#include <stdint.h>

// Definimos __atomic_test_and_set usando __sync_lock_test_and_set de GCC
int __atomic_test_and_set(volatile void *ptr, int memorder) {
    // reinterpretamos ptr como un int y hacemos la operación atómica
    return __sync_lock_test_and_set((volatile int *)ptr, 1);
}

// Definimos __atomic_clear usando __sync_lock_release
void __atomic_clear(volatile void *ptr, int memorder) {
    __sync_lock_release((volatile int *)ptr);
}
