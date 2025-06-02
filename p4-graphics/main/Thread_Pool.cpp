/**
 * @file Thread_Pool.cpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of the Thread_Pool class for managing a pool of threads.
 * @details The Thread_Pool class provides a way to manage a pool of threads that can execute tasks concurrently.
 * @version 1.0
 * @date 2025-06-02
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

#include "Thread_Pool.hpp"
#include <iostream>

namespace Ragot
{
    Thread_Pool & thread_pool = Thread_Pool::instance ();

    void Thread_Pool::thread_function (std::stop_token stop_token)
    {
        while (not stop_token.stop_requested())
        {
            auto task = tasks.get();
            if (task.has_value())
            {
                std::cout << "Thread " << std::this_thread::get_id() << " executing task..." << std::endl;
                task.value ()(stop_token);
            }
        }
    }
}
