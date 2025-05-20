//
//  Sync_Queue.hpp
//  RayTracerEngineApp
//
//  Created by Andrés Ragot on 7/5/25.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace Ragot
{
    template<typename T>
    class Sync_Queue
    {
    public:
        using value_type = T;
        
    public:
        Sync_Queue () = default;
       ~Sync_Queue ()
       {
            close ();
       }
        
        Sync_Queue (const Sync_Queue &) = delete;
        Sync_Queue & operator=(const Sync_Queue &) = delete;
        
    private:
        std::queue<T> queue;
        mutable std::mutex mutex;
        std::condition_variable condition;
        bool closed = false;
        
    public:
        std::optional < value_type > get()
        {
//            std::cout << " Sync_Queue::get()" << std::endl;
        
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
        
        void push (const value_type & value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (not closed)
            {
                queue.push(value);
                condition.notify_one ();
            }
        }
        
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
        
        value_type & back ()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.back();
        }
        
        void close ()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                closed = true;
            }
            condition.notify_all();
        }
        
        void clear ()
        {
            std::queue < value_type >  empty;
            std::lock_guard < std::mutex > lock(mutex);
            queue.swap(empty);
        }
        
        void swap (Sync_Queue & other)
        {
            std::lock(mutex, other.mutex);
            std::lock_guard<std::mutex> lock1(this->mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(other.mutex, std::adopt_lock);
            queue.swap(other.queue);
        }
        
        bool empty () const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty ();
        }
            
        size_t size () const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size ();
        }
        
    };
}
