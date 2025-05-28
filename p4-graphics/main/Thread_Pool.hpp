/**
 * @todo hacer que la queue sea thread safe
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

    class Thread_Pool
    {
    public:
        using Task = std::function <  void (std::stop_token) >;
        std::binary_semaphore sem_mesh_ready {0};
        std::binary_semaphore sem_render_done {0};
    
    private:
        Sync_Queue < Task > tasks;
        
        std::vector < std::unique_ptr < std::jthread > > threads;
        
        std::atomic < bool > started;

    public:
        static Thread_Pool & instance ()
        {
            static Thread_Pool instance;
            return instance;
        }
    
    private:
        Thread_Pool()
        {
            auto cores = std::thread::hardware_concurrency();
            
            std::cout << "Thread pool initialized with " << cores << " threads." << std::endl;
            
            if (cores == 0)
            {
                cores = 2;
            }
            
            threads.resize (cores);
            
            started = false;
        }
        
        ~Thread_Pool()
        {
            sem_mesh_ready.release();
            sem_render_done.release();

            if (started)
            {
                stop();
            }
            
        }
        
        Thread_Pool (Thread_Pool & ) = delete;
        Thread_Pool (Thread_Pool &&) = delete;
        
        Thread_Pool & operator = (Thread_Pool & ) = delete;
        Thread_Pool & operator = (Thread_Pool &&) = delete;
        
    public:
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
        void thread_function (std::stop_token);
        
    };
    
    extern Thread_Pool & thread_pool;
}
