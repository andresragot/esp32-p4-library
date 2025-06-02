/**
 * @file Sync_Queue.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief Implementation of a synchronized queue for thread-safe operations.
 * @details The Sync_Queue class provides a thread-safe queue implementation using mutexes and condition variables.
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

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace Ragot
{
    /**
     * @class Sync_Queue
     * @brief A thread-safe queue implementation.
     * 
     * This class provides a synchronized queue that allows multiple threads to safely push and pop elements.
     * It uses mutexes and condition variables to ensure thread safety.
     */
    template<typename T>
    class Sync_Queue
    {
    public:
        using value_type = T;
        
    public:
        /**
         * @brief Construct a new Sync_Queue object
         */
        Sync_Queue () = default;

        /**
         * @brief Destroy the Sync_Queue object
         * 
         * Closes the queue and releases any resources held by it.
         */
       ~Sync_Queue ()
        {
            close ();
        }
        
        /**
         * @brief Construct a new Sync_Queue object
         * 
         * This constructor is deleted to prevent copying of the Sync_Queue object.
         * It ensures that the queue cannot be copied, which is important for thread safety.
         */
        Sync_Queue (const Sync_Queue &) = delete;

        /**
         * @brief Assignment operator for Sync_Queue
         * 
         * This assignment operator is deleted to prevent copying of the Sync_Queue object.
         * It ensures that the queue cannot be assigned, which is important for thread safety.
         */
        Sync_Queue & operator=(const Sync_Queue &) = delete;
        
    private:
        std::queue<T> queue; ///< The underlying queue to store elements, used for thread-safe operations.
        mutable std::mutex mutex; ///< Mutex to protect access to the queue, ensuring that only one thread can modify the queue at a time.
        std::condition_variable condition; ///< Condition variable to notify waiting threads when elements are available in the queue or when the queue is closed.
        bool closed = false; ///< Flag to indicate whether the queue is closed, preventing further pushes and allowing threads to exit gracefully when the queue is empty.
        
    public:
        /**
         * @brief Retrieves an element from the queue.
         * 
         * This method blocks until an element is available or the queue is closed.
         * If the queue is closed and empty, it returns std::nullopt.
         * 
         * @return std::optional<value_type> The retrieved element or std::nullopt if the queue is closed and empty.
         */
        std::optional < value_type > get()
        {        
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] { return closed || !queue.empty(); });
            if (not closed)
            {
                value_type value = queue.front();
                queue.pop();
                return value;
            }
            return std::nullopt;
        }
        
        /**
         * @brief Pushes an element into the queue.
         * 
         * This method adds an element to the queue if it is not closed.
         * It notifies one waiting thread that an element is available.
         * 
         * @param value The value to be pushed into the queue.
         */
        void push (const value_type & value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (not closed)
            {
                queue.push(value);
                condition.notify_one ();
            }
        }
        
        /**
         * @brief Pushes an element into the queue.
         * 
         * This method adds an element to the queue if it is not closed.
         * It notifies one waiting thread that an element is available.
         * 
         * @param value The value to be pushed into the queue.
         */
        template < typename ...ARGUMENTS >
        void push (ARGUMENTS && ...arguments)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (not closed)
            {
                queue.push (std::forward < ARGUMENTS > (arguments)...);
                condition.notify_one ();
            }
        }
        
        /**
         * @brief Emplaces an element into the queue.
         * 
         * This method constructs an element in place and adds it to the queue if it is not closed.
         * It notifies one waiting thread that an element is available.
         * 
         * @tparam ARGUMENTS The types of arguments used to construct the element.
         * @param arguments The arguments used to construct the element.
         */
        template < typename ...ARGUMENTS >
        void emplace (ARGUMENTS && ...arguments)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (not closed)
            {
                queue.emplace(std::forward<ARGUMENTS>(arguments)...);
                condition.notify_one ();
            }
        }
        
        /**
         * @brief Retrieves the front element of the queue without removing it.
         * 
         * This method returns a reference to the front element of the queue.
         * It is not thread-safe and should be used with caution.
         * 
         * @return value_type& A reference to the front element of the queue.
         */
        value_type & back ()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.back();
        }
        
        /**
         * @brief Closes the queue.
         * 
         * This method sets the closed flag to true and notifies all waiting threads.
         * After closing, no more elements can be pushed into the queue.
         */
        void close ()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                closed = true;
            }
            condition.notify_all();
        }
        
        /**
         * @brief Clears the queue.
         * 
         * This method removes all elements from the queue and resets it to an empty state.
         * It is thread-safe and can be called while other threads are accessing the queue.
         */
        void clear ()
        {
            std::queue < value_type >  empty;
            std::lock_guard < std::mutex > lock(mutex);
            queue.swap(empty);
        }
        
        /**
         * @brief Swaps the contents of this queue with another queue.
         * 
         * This method swaps the contents of this queue with another Sync_Queue.
         * It locks both mutexes to ensure thread safety during the swap operation.
         * 
         * @param other The other Sync_Queue to swap with.
         */
        void swap (Sync_Queue & other)
        {
            std::lock(mutex, other.mutex);
            std::lock_guard<std::mutex> lock1(this->mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(other.mutex, std::adopt_lock);
            queue.swap(other.queue);
        }
        
        /**
         * @brief Checks if the queue is empty.
         * 
         * This method checks if the queue is empty by acquiring a lock on the mutex.
         * It returns true if the queue is empty, false otherwise.
         * 
         * @return true if the queue is empty, false otherwise.
         */
        bool empty () const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty ();
        }
           
        /**
         * @brief Gets the size of the queue.
         * 
         * This method returns the number of elements in the queue by acquiring a lock on the mutex.
         * It is thread-safe and can be called while other threads are accessing the queue.
         * 
         * @return size_t The number of elements in the queue.
         */
        size_t size () const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size ();
        }
    };
}
