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
 
#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <cassert>
#include <future>
#include "Sync_Queue.hpp"
#include <iostream>
#include <functional>
#include <semaphore>

namespace Ragot
{
    /**
     * @class Thread_Pool
     * @brief A thread pool for managing concurrent tasks.
     * 
     * This class provides a thread pool that can execute tasks concurrently using a specified number of threads.
     * It allows submitting tasks and handles synchronization using semaphores.
     */
    class Thread_Pool
    {
    public:
        using Task = std::function <  void (std::stop_token) >;
        std::binary_semaphore sem_mesh_ready {0}; ///< Semaphore to signal that the mesh is ready for rendering.
        std::binary_semaphore sem_render_done {0}; ///< Semaphore to signal that the rendering is done.
    
    private:
        /**
         * @brief A synchronized queue for managing tasks in the thread pool.
         * 
         * This queue is used to store tasks that need to be executed by the threads in the pool.
         * It provides thread-safe operations for pushing and popping tasks.
         */
        Sync_Queue < Task > tasks;
        
        /**
         * @brief A vector of unique pointers to jthread objects representing the threads in the pool.
         * 
         * This vector holds the threads that will execute tasks concurrently.
         * Each thread is managed by a unique pointer to ensure proper resource management.
         */
        std::vector < std::unique_ptr < std::jthread > > threads;
        
        std::atomic < bool > started; ///< Flag indicating whether the thread pool has been started or not.

    public:
        /**
         * @brief Singleton instance of the Thread_Pool class.
         * 
         * This method provides access to the singleton instance of the Thread_Pool.
         * It ensures that only one instance of the thread pool exists throughout the application.
         * 
         * @return Thread_Pool& Reference to the singleton instance of the Thread_Pool.
         */
        static Thread_Pool & instance ()
        {
            static Thread_Pool instance;
            return instance;
        }
    
    private:
        /**
         * @brief Private constructor for the Thread_Pool class.
         * 
         * Initializes the thread pool with the number of threads equal to the number of hardware cores available.
         * If the number of cores is zero, it defaults to two threads.
         */
        Thread_Pool()
        {
            auto cores = std::thread::hardware_concurrency();
            
            std::cout << "Thread pool initialized with " << cores << " threads." << std::endl;
            
            if (cores == 0)
            {
                cores = 2; // Default to 2 threads if hardware concurrency is not available
            }
            
            threads.resize (cores);
            
            started = false;
        }
        
        /**
         * @brief Destructor for the Thread_Pool class.
         * 
         * Cleans up resources used by the thread pool, releases semaphores, and stops all threads if they are running.
         */
        ~Thread_Pool()
        {
            sem_mesh_ready.release();
            sem_render_done.release();

            if (started)
            {
                stop();
            }
            
        }
        
        /**
         * @brief Construct a new Thread_Pool object (Deleted).
         * This constructor is deleted to prevent copying of the Thread_Pool object. 
         */
        Thread_Pool (Thread_Pool & ) = delete;

        /**
         * @brief Move constructor for the Thread_Pool class (Deleted).
         * This move constructor is deleted to prevent moving of the Thread_Pool object.
         */
        Thread_Pool (Thread_Pool &&) = delete;
        
        /**
         * @brief Assignment operator for the Thread_Pool class (Deleted).
         * This assignment operator is deleted to prevent copying of the Thread_Pool object.
         */
        Thread_Pool & operator = (Thread_Pool & ) = delete;

        /**
         * @brief Move assignment operator for the Thread_Pool class (Deleted).
         * This move assignment operator is deleted to prevent moving of the Thread_Pool object.
         */
        Thread_Pool & operator = (Thread_Pool &&) = delete;
        
    public:

        /**
         * @brief Starts the thread pool by creating threads and binding them to the thread_function.
         * 
         * This method initializes the threads in the pool and starts them, allowing them to execute tasks concurrently.
         * It asserts that the thread pool has not already been started to prevent multiple initializations.
         */
        void start ()
        {
            assert (not started);
            
            std::cout << "Starting thread pool..." << std::endl;
            
            for (auto & thread : threads)
            {
                thread = std::make_unique < std::jthread > ( std::bind (&Thread_Pool::thread_function, this, std::placeholders::_1) );
            }
            
            started = true;
        }
        
        /**
         * @brief Stops the thread pool by closing the task queue and requesting all threads to stop.
         * 
         * This method stops the thread pool, ensuring that all threads are requested to stop gracefully.
         * It asserts that the thread pool has been started before attempting to stop it.
         */
        void stop ()
        {
            assert (started == true);
            started = false;
            
            tasks.close ();
            for (auto & thread : threads)
            {
                thread->request_stop();
            }

            
            threads.clear();
                    
        }
        
        /**
         * @brief Submits a task to the thread pool for execution.
         * 
         * This method allows submitting a task to the thread pool, which will be executed by one of the threads.
         * It returns a future that can be used to retrieve the result of the task once it is completed.
         * 
         * @tparam F The type of the function to be executed.
         * @tparam Args The types of the arguments to be passed to the function.
         * @param f The function to be executed.
         * @param args The arguments to be passed to the function.
         * @return std::future<std::invoke_result_t<F, Args...>> A future representing the result of the task.
         */
        template<typename F, typename... Args>
        std::future < std::invoke_result_t < F, Args... > > submit (F && f, Args && ... args)
        {
            std::cout << "submit" << std::endl;
            
            using ReturnType = std::invoke_result_t < F, Args... >;

            auto task_ptr = std::make_shared < std::packaged_task < ReturnType() > > (
                std::bind (std::forward < F > (f), std::forward < Args > (args)...)
            );

            std::future<ReturnType> res = task_ptr->get_future();

            tasks.push ([task_ptr](std::stop_token) {
                (*task_ptr)();
            });

            return res;
        }
        
        /**
         * @brief Submits a task to the thread pool for execution with a stop token.
         * 
         * This method allows submitting a task to the thread pool, which will be executed by one of the threads.
         * It accepts a stop token that can be used to request cancellation of the task.
         * It returns a future that can be used to retrieve the result of the task once it is completed.
         * 
         * @tparam F The type of the function to be executed.
         * @tparam Args The types of the arguments to be passed to the function.
         * @param f The function to be executed.
         * @param args The arguments to be passed to the function.
         * @return std::future<std::invoke_result_t<F, std::stop_token, Args...>> A future representing the result of the task.
         */
        template<typename F, typename... Args>
        std::future<std::invoke_result_t<F, std::stop_token, Args...>>
        submit_with_stop(F&& f, Args&&... args)
        {
            std::cout << "submit_with_stop" << std::endl;
            
            using ReturnType = std::invoke_result_t<F, std::stop_token, Args...>;

            auto task_ptr = std::make_shared<std::packaged_task<ReturnType(std::stop_token)>>(
                std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
            );
            std::future<ReturnType> res = task_ptr->get_future();

            tasks.push([task_ptr](std::stop_token tok) {
                (*task_ptr)(tok);
            });

            return res;
        }

        
    private:
        /**
         * @brief The function executed by each thread in the thread pool.
         * 
         * This function continuously retrieves tasks from the task queue and executes them.
         * It uses a stop token to allow threads to gracefully exit when requested.
         * 
         * @param stop_token The stop token used to request cancellation of the task.
         */
        void thread_function (std::stop_token);
        
    };
    
    extern Thread_Pool & thread_pool;
}
